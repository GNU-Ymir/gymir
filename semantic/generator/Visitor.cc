#include <ymir/semantic/generator/Visitor.hh>
#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/semantic/symbol/Struct.hh>
#include <ymir/semantic/symbol/Class.hh>
#include <ymir/semantic/symbol/Enum.hh>

#include <ymir/utils/Match.hh>
#include <ymir/global/Core.hh>
#include <ymir/global/State.hh>
#include <ymir/syntax/visitor/Keys.hh>

#include "toplev.h"
using namespace global;

namespace semantic {

    namespace generator {

	using namespace generic;

	static GTY(()) vec<tree, va_gc> *globalDeclarations;

	const ulong	Visitor::ReturnWithinCatch::NONE   = 0;
	const ulong	Visitor::ReturnWithinCatch::RETURN = 1;
	const ulong	Visitor::ReturnWithinCatch::BREAK  = 2;
	const ulong	Visitor::ReturnWithinCatch::THROW  = 3;

	std::set <std::string> Visitor::__definedFrame__;
	
	Visitor::Visitor () :
	    _globalContext (Tree::empty ()),
	    _currentContext (Tree::empty ())
	{}

	Visitor Visitor::init () {
	    return Visitor ();
	}

	void Visitor::finalize () {
	    if (this-> _globalInitialiser.size () != 0) {
		generateGlobalInitFrame ();
	    }
	    
	    int len = vec_safe_length (globalDeclarations);
	    tree * addr = vec_safe_address (globalDeclarations);
	    for (int i = 0 ; i < len ; i++) {
		tree decl = addr [i];
		wrapup_global_declarations (&decl, 1);
	    }
	}
	
	void Visitor::generate (const Generator & gen) {
	    match (gen) {
		of (GlobalVar, var,
		    generateGlobalVar (var);
		    return;
		);

		of (Frame, frame,
		    generateFrame (frame);
		    return;
		);

		of (Class, cl,
		    generateVtable (cl.getClassRef ());
		    return;
		);
	    }
	   	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	}	

	Tree Visitor::generateType (const Generator & gen) {
	    Tree type = Tree::empty ();
	    match (gen) {
		of (Integer, i,
		    type = Tree::intType (i.getSize (), i.isSigned ());
		)		

		else of (Void, v ATTRIBUTE_UNUSED,
		    type = Tree::voidType ();
		)

		else of (Bool, b ATTRIBUTE_UNUSED,
		    type = Tree::boolType ();
		)
		
		else of (Float, f,
		    type = Tree::floatType (f.getSize ());
		)

		else of (Char, c,
		    type = Tree::charType (c.getSize ());
		)

		else of (Array, array,
		    type = Tree::staticArray (generateType (array.getInners () [0]), array.getSize ());
		)

		else of (Slice, slice,
		    type = Tree::sliceType (generateType (slice.getInners () [0]));
		)

		else of (Tuple, tu, {
			std::vector <Tree> inner;
			for (auto & it : tu.getInners ()) inner.push_back (generateType (it));
			type = Tree::tupleType ({}, inner);
		    }
		)

		else of (TupleClosure, tu, {
			std::vector <Tree> inner;
			for (auto & it : tu.getInners ()) inner.push_back (generateType (it));
			type = Tree::tupleType ({}, inner);
		    }
		)
			 
		else of (StructRef, st, {
			static std::set <std::string> current;
			if (current.find (st.prettyString ()) == current.end ()) { // To prevent infinite loop for inner type validation
			    current.emplace (st.prettyString ());
			    auto gen = st.getRef ().to <semantic::Struct> ().getGenerator ();
			    std::vector <Tree> inner;
			    std::vector <std::string> fields;
			    for (auto & it : gen.to <generator::Struct> ().getFields ()) {
				inner.push_back (generateType (it.to <generator::VarDecl> ().getVarType ()));
				fields.push_back (it.to <generator::VarDecl> ().getName ());
			    }

			    if (inner.size () == 0) {
				inner.push_back (Tree::charType (8));
				fields.push_back ("_");
			    }
			    
			    type = Tree::tupleType (st.getRef ().getRealName (), fields, inner, st.getRef ().to <semantic::Struct> ().isUnion (), st.getRef ().to <semantic::Struct> ().isPacked ());
			    current.erase (st.prettyString ());
			} else return Tree::voidType ();
		    }
		)
			 
		else of (ClassRef, cl, {
			type = generateClassType (cl);
		    }
		)
			 
		else of (EnumRef, en, {
			auto _type = en.getRef ().to <semantic::Enum> ().getGenerator ().to <generator::Enum> ().getType ();
			type = generateType (_type);			
		    })

		else of (Pointer, pt, {
			auto inner = generateType (pt.getInners ()[0]);
			type = Tree::pointerType (inner);			    
		    })
		else of (ClassPtr, pt, {
			auto inner = generateType (pt.getInners ()[0]);
			type = Tree::pointerType (inner);
		    })
		else of (Range, rg, {
		       std::vector <Tree> inner;
		       inner.push_back (generateType (rg.getInners ()[0]));
		       inner.push_back (generateType (rg.getInners ()[0]));
		       if (rg.getInners () [0].is <Float> ())
			   inner.push_back (generateType (rg.getInners ()[0]));
		       else if (rg.getInners ()[0].is <Integer> ()) {
			   inner.push_back (generateType (Integer::init (rg.getInners ()[0].getLocation (), rg.getInners () [0].to <Integer> ().getSize (), true)));
		       } else if (rg.getInners ()[0].is <Char> ()) {
			   inner.push_back (generateType (Integer::init (rg.getInners ()[0].getLocation (), rg.getInners () [0].to <Char> ().getSize (), true)));
		       } else
			   Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");

		       inner.push_back (generateType (Bool::init (rg.getLocation ())));
		       type = Tree::tupleType ({Range::FST_NAME, Range::SCD_NAME, Range::STEP_NAME, Range::FULL_NAME}, inner);
		   }
	       ) else of (FuncPtr, fn, {
		       auto retType = generateType (fn.getReturnType ());
		       std::vector <Tree> params;
		       for (auto & it : fn.getParamTypes ())
			   params.push_back (generateType (it));
		       type = Tree::pointerType (
			   Tree::functionType (retType, params)
		       );
		   }
	       ) else of (Closure, c, {
		       std::vector <Tree> inner;
		       std::vector <std::string> fields = c.getNames ();
		       for (auto & it : c.getInners ()) {
			   inner.push_back (generateType (it));
		       }
		       type = Tree::tupleType (fields, inner);
		   }
	       ) else of (Delegate, d, {
		       std::vector <Tree> inner;
		       inner.push_back (Tree::pointerType (Tree::voidType ()));
		       if (d.getInners ()[0].is <FrameProto> ()) {
			   auto & proto = d.getInners ()[0];
			   auto params = proto.to <FrameProto> ().getParameters ();
			   auto ret = proto.to <FrameProto> ().getReturnType ();
			   std::vector <Generator> paramTypes;
			   for (auto & it : params) {
			       paramTypes.push_back (it.to <generator::ProtoVar> ().getType ());
			   }
	    
			   inner.push_back (generateType (FuncPtr::init (proto.getLocation (), ret, paramTypes)));
		       } else {
			   inner.push_back (generateType (d.getInners () [0]));
		       }
		       type = Tree::tupleType ({}, inner); // delegate are unnamed tuple .0 closure, .1 funcptr
		   }
	       ) else of (Option, o, {
		       auto inner = generateType (o.getInners ()[0]);
		       auto errorType = generateType (o.getInners ()[1]);
		       auto boolType = Tree::boolType ();
		       std::vector <std::string> names;
		       names.push_back (Option::TYPE_FIELD);
		       if (!o.getInners ()[0].is<Void> ())
			   names.push_back (Option::VALUE_FIELD);
		       
		       names.push_back (Option::ERROR_FIELD);
		       
		       std::vector <Tree> unions;
		       if (!o.getInners ()[0].is <Void> ())
			   unions.push_back (inner);
		       
		       unions.push_back (errorType);
		       
		       type = Tree::optionType (names, {boolType}, unions);
		   }
	       );
	    }

	    if (type.isEmpty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");

	    if (gen.to <Type> ().isRef ())
		type = Tree::pointerType (type);
	    
	    return type;
	}

	Tree Visitor::generateClassType (const ClassRef & ref) {	    
	    static std::set <std::string> current;
	    if (current.find (ref.prettyString ()) == current.end ()) { // To prevent infinite loop in type validation
		current.emplace (ref.prettyString ());
		auto gen = ref.getRef ().to <semantic::Class> ().getGenerator ();
		std::vector <Tree> inner;
		std::vector <std::string> fields;
		fields.push_back ("#_vtable");
		fields.push_back ("#_monitor");
		inner.push_back (Tree::pointerType (Tree::pointerType (Tree::voidType ()))); // vtable
		inner.push_back (Tree::pointerType (Tree::pointerType (Tree::voidType ()))); // monitor
		
		for (auto & it : gen.to <generator::Class> ().getFields ()) {
		    inner.push_back (generateType (it.to <generator::VarDecl> ().getVarType ()));
		    fields.push_back (it.to <generator::VarDecl> ().getName ());
		}
		
		auto type = // Tree::pointerType (
		    Tree::tupleType (fields, inner);
		    //);
		current.erase (ref.prettyString ());
		type.getFieldOffsets (ref.getLocation ());
		return type;
	    } else return // Tree::pointerType (
		       Tree::voidType ()//);
		       ;
	}
		
	Tree Visitor::generateInitValueForType (const Generator & type) {
	    match (type) {

		of (Integer, i,
		    return Tree::buildIntCst (i.getLocation (), Integer::INIT, generateType (type));
		);

		of (Void, v ATTRIBUTE_UNUSED,
		    Ymir::Error::halt ("%(r) - reaching impossible point - value for a void", "Critical");
		);

		of (Bool, b,
		    return Tree::buildBoolCst (b.getLocation (), Bool::INIT);
		);

		of (Float, f,
		    return Tree::buildFloatCst (f.getLocation (), Float::INIT, generateType (type));
		);

		of (Char, c,
		    return Tree::buildCharCst (c.getLocation (), Char::INIT, generateType (type));
		);

		of (Array, a,
		    return Tree::constructIndexed0 (
			a.getLocation (),
			generateType (type),
			generateInitValueForType (a.getInners () [0]),
			a.getSize ()
		    );
		);

		of (Slice, s,
		    return Tree::constructField (
			s.getLocation (),
			generateType (type),
			{Slice::LEN_NAME, Slice::PTR_NAME},
			{
			    Tree::buildSizeCst (Integer::INIT),
			    Tree::buildIntCst (s.getLocation (), Integer::INIT, Tree::pointerType (generateType (s.getInners () [0])))
			}
		    );
		);
	    }	    

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Tree::empty ();
	}
	
	
	void Visitor::generateGlobalVar (const GlobalVar & var) {
	    auto type = generateType (var.getType ());
	    auto name = Mangler::init ().mangleGlobalVar (var);
	    
	    Tree decl = Tree::varDecl (var.getLocation (), name, type);
	    if (!var.getValue ().isEmpty ()) {
		enterBlock (var.getLocation ().toString ());
		auto value = castTo (var.getType (), var.getValue ());
		if (stackVarDeclChain.back ().begin ().current.isEmpty ()) {
		    decl.setDeclInitial (value);
		} else {
		    this-> _globalInitialiser.emplace (var.getUniqId (), std::pair<generator::Generator, generator::Generator> (var.getType (), var.getValue ()));
		    }
		quitBlock (var.getLocation (), Tree::empty (), var.getLocation ().toString ());		
	    } 
 
	    decl.isStatic (true);
	    decl.isUsed (true);
	    if (var.isExternal()) 
		decl.isExternal (true);
	    else decl.isExternal (false);
	    
	    decl.isPreserved (true);
	    decl.isPublic (true);
	    decl.setDeclContext (getGlobalContext ());
	   
	    vec_safe_push (globalDeclarations, decl.getTree ());
	    insertGlobalDeclarator (var.getUniqId (), decl);
	}

	Tree Visitor::generateGlobalConstant (const GlobalConstant & cst) {
	    static std::map <std::string, generic::Tree> __globalConstant__;
	    auto name = Mangler::init ().mangleGlobalConstant (cst);
	    auto res = __globalConstant__.find (name);
	    if (res != __globalConstant__.end ()) return res-> second;
	    
	    auto type = generateType (cst.getType ());
	    Tree decl = Tree::varDecl (cst.getLocation (), name, type);
	    if (cst.getValue ().is <StringValue> ()) {
		auto & val = cst.getValue ().to <StringValue> ();
		std::vector <Tree> params;
		auto type = generateType (val.getType ());
		auto inner = generateType (val.getType ().to <Type> ().getInners ()[0]);
		auto ptr = cst.getValue ().to <StringValue> ().getValue ();
		auto i = 0;
		// Utf-32
		while (i < (int) ptr.size ()) {
		    auto data = (int*) (ptr.data () + i);
		    auto value = Tree::buildCharCst (cst.getLocation (), *data, inner);
		    params.push_back (value);
		    i += 4;
		}		
		decl.setDeclInitial (Tree::constructIndexed (val.getLocation (), type, params));
		
	    } else {
		auto value = castTo (cst.getType (), cst.getValue ());
		decl.setDeclInitial (value);
	    }
	    
	    decl.isWeak (true);
	    decl.isStatic (true);
	    decl.isUsed (true);
	    decl.isExternal (false);
	    decl.isPreserved (true);
	    decl.isPublic (true);	    
	    decl.setDeclContext (getGlobalContext ());
	    
	    vec_safe_push (globalDeclarations, decl.getTree ());
	    __globalConstant__.emplace (name, decl);
	    
	    return decl;
	}

	Tree Visitor::generateTypeInfoClass (const Generator & classType) {
	    static std::map <std::string, generic::Tree> __globalConstant__;
	    auto name = Mangler::init ().mangleTypeInfo (classType.to <ClassRef> ());
	    auto res = __globalConstant__.find (name);
	    if (res != __globalConstant__.end ()) return res-> second;

	    auto typeInfo = classType.to<ClassRef> ().getRef ().to<semantic::Class> ().getTypeInfo ();
	    // auto gen = classType.to<ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
	    Tree ancestorSlice (Tree::empty ());
	    	    
	    if (!classType.to <ClassRef> ().getAncestor ().isEmpty ()) {
		auto ancestor = generateTypeInfoClass (classType.to<ClassRef> ().getAncestor ());
		ancestorSlice = Tree::constructField (
		    classType.getLocation (),
		    generateType (typeInfo.to <StructCst> ().getTypes ()[2]),
		    {Slice::LEN_NAME, Slice::PTR_NAME},
		    {
			Tree::buildSizeCst (1),
			    Tree::buildAddress (classType.getLocation (), ancestor, Tree::pointerType (Tree::voidType ()))
			    }
		);		    
	    } else {
		ancestorSlice = Tree::constructField (
		    classType.getLocation (),
		    generateType (typeInfo.to <StructCst> ().getTypes ()[2]),
		    {Slice::LEN_NAME, Slice::PTR_NAME},
		    {
			Tree::buildSizeCst (Integer::INIT),
			    Tree::buildIntCst (classType.getLocation (), Integer::INIT, Tree::pointerType (Tree::voidType ()))
			    }
		);
	    }

	    auto declName = generateValue (typeInfo.to <StructCst> ().getParameters() [3]);
	    // auto slcType = generateType (typeInfo.to <StructCst> ().getTypes () [3]);	    
	    // auto slcName = Tree::constructField (
	    // 	classType.getLocation (),
	    // 	slcType,
	    // 	{Slice::LEN_NAME, Slice::PTR_NAME},
	    // 	{
	    // 	    declName.getField (Slice::LEN_NAME),
	    // 		declName.getField (Slice::PTR_NAME)
	    // 		}
	    // );
	    
	    std::vector <Tree> params = {
	    	castTo (typeInfo.to <StructCst> ().getTypes () [0], typeInfo.to <StructCst> ().getParameters() [0]),
		castTo (typeInfo.to <StructCst> ().getTypes () [1], typeInfo.to <StructCst> ().getParameters() [1]),
	    	ancestorSlice,
		declName.getDeclInitial ()
	    };

	    std::vector <Tree> types = {
		params [0].getType (),
		params [1].getType (),
		params [2].getType (),
		params [3].getType ()
	    };
	    
	    auto typeValue = Tree::constructField (
	    	classType.getLocation (),
		Tree::tupleType ({}, types),
	    	{},
	    	params
	    );
	    
	    Tree decl = Tree::varDecl (classType.getLocation (), name, typeValue.getType ());
	    decl.setDeclInitial (typeValue);
	    decl.isStatic (true);
	    decl.isUsed (true);
	    decl.isExternal (false);
	    decl.isPreserved (true);
	    decl.isPublic (true);
	    decl.isWeak (true);
	    decl.setDeclContext (getGlobalContext ());	 

	    vec_safe_push (globalDeclarations, decl.getTree ());
	    __globalConstant__.emplace (name, decl);
	    
	    return decl;	    
	}

	Tree Visitor::generateVtable (const Generator & classType) {
	    static std::map <std::string, generic::Tree> __globalConstant__;
	    auto name = Mangler::init ().mangleVtable (classType.to<ClassRef> ());
	    auto res = __globalConstant__.find (name);
	    if (res != __globalConstant__.end ()) return res-> second;

	    std::vector<Tree> params;
	    auto & classGen = classType.to <ClassRef> ().getRef ().to <semantic::Class> ().getGenerator ();
	    params.push_back (Tree::buildAddress (classType.getLocation (), generateTypeInfoClass (classType), Tree::pointerType (Tree::voidType ())));
	    
	    for (auto & it : classGen.to <generator::Class> ().getVtable ()) {
		if (it.to <MethodProto> ().isEmptyFrame ()) {
		    params.push_back (Tree::buildPtrCst (it.getLocation (), 0));
		} else {
		    params.push_back (generateValue (it));
		}
	    }	    
	    
	    auto vtableType = Tree::staticArray (Tree::pointerType (Tree::voidType ()), params.size ());
	    auto vtableValue = Tree::constructIndexed (classType.getLocation (), vtableType, params);
		
	    Tree decl = Tree::varDecl (classType.getLocation (), name, vtableType);
	    decl.setDeclInitial (vtableValue);
	    decl.isStatic (true);
	    decl.isUsed (true);
	    decl.isExternal (false);
	    decl.isPreserved (true);
	    decl.isPublic (true);
	    decl.isWeak (true);
	    decl.setDeclContext (getGlobalContext ());	 

	    vec_safe_push (globalDeclarations, decl.getTree ());
	    __globalConstant__.emplace (name, decl);
	    
	    return decl;
	}
	
	
	void Visitor::generateMainCall (const lexing::Word & loc, bool isVoid, const std::string & mainName) {
	    auto argcT = Tree::intType (64, false);
	    auto argvT = Tree::pointerType (Tree::pointerType (Tree::charType (8)));
	    std::vector <Tree> args = {argcT, argvT};
	    
	    auto ret = Tree::intType (32, true);

	    Tree fnType = Tree::functionType (ret, args);
	    Tree fn_decl = Tree::functionDecl (loc, Keys::MAIN, fnType);
	    auto asmName = Keys::MAIN;
	    fn_decl.asmName (asmName);

	    setCurrentContext (fn_decl);
	    enterFrame ();

	    auto argc = generateParamVar ("argc", argcT);
	    auto argv = generateParamVar ("argv", argvT);
	    std::list <Tree> argsList = {argc, argv};

	    fn_decl.setDeclArguments (argsList);

	    enterBlock (loc.toString ());
	    auto resultDecl = Tree::resultDecl (loc, ret);
	    fn_decl.setResultDecl (resultDecl);
	    TreeStmtList list = TreeStmtList::init ();
	    std::string name;
	    if (global::State::instance ().isDebugActive ()) {
		name = global::CoreNames::get (RUN_MAIN_DEBUG);
	    } else name = global::CoreNames::get (RUN_MAIN);
	    

	    Tree mainRet = ret;
	    if (isVoid) mainRet = Tree::voidType ();

	    auto proto = Tree::buildFrameProto (loc, mainRet, mainName, {});
	    
	    auto call = Tree::buildCall (
		loc,
		mainRet,
		name,
		{argc, argv, proto}
	    );

	    if (!isVoid)
		list.append (Tree::returnStmt (loc, resultDecl, call));
	    else {
		list.append (call);
		list.append (Tree::returnStmt (
		    loc, resultDecl,
		    Tree::buildIntCst (loc, (ulong) 0, ret)
		));
	    }
	    
	    Tree value = list.toTree ();

	    auto fnTree = quitBlock (loc, value, loc.toString ());
	    auto fnBlock = fnTree.block;
	    fnBlock.setBlockSuperContext (fn_decl);

	    fn_decl.setDeclInitial (fnBlock);
	    fn_decl.setDeclSavedTree (fnTree.bind_expr);
	    fn_decl.isExternal (false);
	    fn_decl.isPreserved (true);
	    fn_decl.isPublic (true);
	    fn_decl.isStatic (true);


	    Tree::gimplifyFunction (fn_decl);
	    Tree::finalizeFunction (fn_decl);
	    setCurrentContext (Tree::empty ());
	    quitFrame ();	    	    
	}

	void Visitor::generateGlobalInitFrame () {
	    Tree ret = Tree::voidType ();
	    std::vector <Tree> args;

	    Tree fnType = Tree::functionType (ret, args);
	    Tree fn_decl = Tree::functionDecl (lexing::Word::eof (), "_GLOBAL_", fnType);
	    setCurrentContext (fn_decl);
	    enterFrame ();

	    enterBlock ("__GLOBAL__");
	    auto resultDecl = Tree::resultDecl (lexing::Word::eof (), ret);
	    fn_decl.setResultDecl (resultDecl);
	    TreeStmtList list (TreeStmtList::init ());
	    
	    for (auto & it : this-> _globalInitialiser) {
		auto value = castTo (it.second.first, it.second.second);
		list.append (value.getList ());
		auto decl = this-> _globalDeclarators.find (it.first);
		list.append (Tree::affect (stackVarDeclChain.back (), getCurrentContext (), it.second.first.getLocation (), decl-> second, value.getValue ()));
	    }

	    auto fnTree = quitBlock (lexing::Word::eof (), list.toTree (), "__GLOBAL__");
	    auto fnBlock = fnTree.block;
	    fnBlock.setBlockSuperContext (fn_decl);

	    fn_decl.setDeclInitial (fnBlock);
	    fn_decl.setDeclSavedTree (fnTree.bind_expr);
	    fn_decl.isExternal (false);
	    fn_decl.isPreserved (true);
	    fn_decl.isWeak (false);

	    fn_decl.isPublic (false);
	    fn_decl.isStatic (true);
	    fn_decl.isGlobalCstr (true);

	    Tree::gimplifyFunction (fn_decl);
	    Tree::finalizeFunction (fn_decl);

	    
	    quitFrame ();
	    setCurrentContext (Tree::empty ());
	}
	
	void Visitor::generateFrame (const Frame & frame) {	    
	    std::vector <Tree> args;
	    for (auto i : Ymir::r (0, args.size ())) {
		args.push_back (generateType (frame.getParams () [i].to<ParamVar> ().getType ()));
	    }

	    Tree ret = generateType (frame.getType ());	    
	    Tree fntype = Tree::functionType (ret, args);
	    Tree fn_decl = Tree::functionDecl (frame.getLocation (), frame.getName (), fntype);
	    auto asmName = Mangler::init ().mangleFrame (frame);
	    fn_decl.asmName (asmName);
	    if (!frame.isWeak () || __definedFrame__.find (asmName) == __definedFrame__.end ())	{
		if (frame.getName () == Keys::MAIN) 
		    generateMainCall (frame.getLocation (), frame.getType ().is <Void> (), asmName);
	    
		setCurrentContext (fn_decl);
		enterFrame ();

		std::list <Tree> arglist;
		for (auto & p : frame.getParams ())
		    arglist.push_back (generateParamVar (p.to<ParamVar> ()));
	    
		fn_decl.setDeclArguments (arglist);
	    
		enterBlock (frame.getLocation ().toString ());
		auto resultDecl = Tree::resultDecl (frame.getLocation (), ret);
		fn_decl.setResultDecl (resultDecl);	       
		
		Tree value (Tree::empty ());
		if (frame.needFinalReturn ()) {
		    TreeStmtList list = TreeStmtList::init ();
		    value = castTo (frame.getType (), frame.getContent ());
		    list.append (value.getList ());
		    value = value.getValue ();

		    list.append (Tree::returnStmt (frame.getLocation (), resultDecl, value));
		    value = list.toTree ();
		} else value = generateValue (frame.getType (), frame.getContent ());
	    
		auto fnTree = quitBlock (lexing::Word::eof (), value, frame.getLocation ().toString ());
		auto fnBlock = fnTree.block;
		fnBlock.setBlockSuperContext (fn_decl);	    
	    
		fn_decl.setDeclInitial (fnBlock);	    
		fn_decl.setDeclSavedTree (fnTree.bind_expr);

		fn_decl.isExternal (false);
		fn_decl.isPreserved (true);
		fn_decl.isWeak (frame.isWeak ());

		fn_decl.isPublic (true);
		fn_decl.isStatic (true);

		Tree::gimplifyFunction (fn_decl);
		Tree::finalizeFunction (fn_decl);

		__definedFrame__.emplace (asmName);
		
		setCurrentContext (Tree::empty ());
		quitFrame ();
	    }
	}
	
	Tree Visitor::generateParamVar (const ParamVar & var) {
	    auto type = generateType (var.getType ());
	    auto name = var.getName ();

	    auto decl = Tree::paramDecl (var.getLocation (), name, type);
	    
	    decl.setDeclContext (getCurrentContext ());
	    decl.setArgType (decl.getType ());
	    decl.isUsed (true);

	    insertDeclarator (var.getUniqId (), decl);
	    
	    return decl;
	}

	Tree Visitor::generateParamVar (const std::string & name, const Tree & type) {
	    auto decl = Tree::paramDecl (lexing::Word::eof (), name, type);
	    
	    decl.setDeclContext (getCurrentContext ());
	    decl.setArgType (decl.getType ());
	    decl.isUsed (true);
	    
	    return decl;
	}

	Tree Visitor::generateValue (const Generator & gen) {
	    auto value = generateValueInner (gen);
	    if (gen.to<Value> ().getType ().to <Type> ().isRef ())
		value = value.toDirect ();
	    return value;
	}

	Tree Visitor::generateValue (const Generator & type, const Generator & gen) {
	    auto value = generateValueInner (gen);
	    if (gen.to <Value> ().getType ().to <Type> ().isRef ()) {
		if (!type.to <Type> ().isRef ())
		    value = value.toDirect ();
	    }
	    return value;
	}

	
	Tree Visitor::generateValueInner (const Generator & gen) {
	    match (gen) {
		of (Block, block,
		    return generateBlock (block);
		)

		else of (Set, set,
		    return generateSet (set);
		)
		
		else of (Fixed, fixed,
		    return generateFixed (fixed);
		)

		else of (BoolValue, b,
		    return generateBool (b);
		)

		else of (FloatValue, f,
		    return generateFloat (f);
		)

		else of (CharValue, c,
		    return generateChar (c);
		)
		
		else of (Affect, aff,
		    return generateAffect (aff);
		)
		
		else of (BinaryInt, i,
		    return generateBinaryInt (i);
		)

		else of (BinaryBool, b,
		    return generateBinaryBool (b);
		)

		else of (BinaryFloat, f,
		    return generateBinaryFloat (f);
		)
			 
		else of (BinaryChar, ch,
		    return generateBinaryChar (ch);
		)
			 
		else of (BinaryPtr, ptr,
		    return generateBinaryPtr (ptr);
		)
			 
		else of (VarRef, var,
		    return generateVarRef (var);
		)

		else of (VarDecl, decl,
		    return generateVarDecl (decl);
		)

		else of (Referencer, _ref,
		    return generateReferencer (_ref);
		)

		else of (Addresser, addr,
		    return generateAddresser (addr);
		)

		else of (Conditional, cond,
		    return generateConditional (cond);
		)

		else of (Loop, loop,
		    return generateLoop (loop);
		)

		else of (Break, br,
		    return generateBreak (br);
		)
		
		else of (ArrayValue, val,
		    return generateArrayValue (val);
		)

		else of (Copier, copy,
		    return generateCopier (copy);
		)

		else of (SizeOf, size,
		    return generateSizeOf (size);
		)
			 
		else of (Aliaser, al,
		    return generateAliaser (al);
		)

		else of (None, none ATTRIBUTE_UNUSED,
		    return Tree::empty ();
		)

		else of (ArrayAccess, access,
		    return generateArrayAccess (access);
		)

		else of (SliceAccess, access,
		    return generateSliceAccess (access);
		)
		
                else of (SliceConcat, sc,
		    return generateSliceConcat (sc);
		)
			     
		else of (UnaryBool, ub,
		    return generateUnaryBool (ub);
		)

		else of (UnaryInt, ui,
		    return generateUnaryInt (ui);
		)

		else of (UnaryFloat, uf,
		    return generateUnaryFloat (uf);
		)

		else of (UnaryPointer, uf,
		    return generateUnaryPointer (uf);
		)

		else of (TupleValue, tu,
		    return generateTupleValue (tu);
		)

		else of (StringValue, str,
		     return generateStringValue (str);
		)
			 
		else of (Call, cl,
		    return generateCall (cl);		
		)
			 
		else of (ClassCst, cs,
		    return generateClassCst (cs);
		)
			 
		else of (FrameProto, pr,
		    return generateFrameProto (pr);		
		)

		else of (ConstructorProto, pr,
		    return generateConstructorProto (pr);
		)

		else of (TupleAccess, acc,
		    return generateTupleAccess (acc);		
		)

		else of (StructAccess, acc,
		   return generateStructAccess (acc);					 
		)

		else of (StructCst, cst,
		    return generateStructCst (cst);	       
		)

		else of (UnionCst, cst,
		    return generateUnionCst (cst);	       
		)

		else of (StructRef, rf ATTRIBUTE_UNUSED,
		    return Tree::empty ();
		)

		else of (Return, rt,
		    return generateReturn (rt);
		)

		else of (RangeValue, rg,
		    return generateRangeValue (rg);
		)

		else of (SliceValue, sl,
		    return generateSliceValue (sl);
		)
			 
		else of (DelegateValue, dg,
		    return generateDelegateValue (dg);
		)
			 
		else of (Cast, cast,
		    return generateCast (cast);
		)

		else of (AtomicLocker, lock,
		    return generateAtomicLocker (lock);
		)

		else of (AtomicUnlocker, lock,
		    return generateAtomicUnlocker (lock);
		)
			 
		else of (ArrayAlloc, alloc,
		    return generateArrayAlloc (alloc);
		)

		else of (NullValue, nl,
		    return generateNullValue (nl);
		)

		else of (UniqValue, uv,
		     return generateUniqValue (uv);
		)

		else of (Throw, th,
		    return generateThrow (th);
		)

		else of (ExitScope, ex,
		    return generateExitScope (ex);
		)

		else of (SuccessScope, succ,
		    return generateSuccessScope (succ);
		)
			 
		else of (GlobalConstant, cst,
		    return generateGlobalConstant (cst);
		)

		else of (VtableAccess, acc,
		    return generateVtableAccess (acc);
		)

		else of (ThrowBlock, bl,
		    return generateThrowBlock (bl);
		)

	        else of (OptionValue, vl,
		    return generateOptionValue (vl);
		);
	    }
	    
	    println (gen.prettyString ());
	    Ymir::Error::halt ("%(r) - reaching impossible point %(y)", "Critical", identify (gen));
	    return Tree::empty ();
	}
	
	Tree Visitor::generateBlock (const Block & block) {
	    if (block.isLvalue ()) return generateLeftBlock (block);
	    
	    Tree var (Tree::empty ());
	    auto varName = Ymir::format ("_b(%)", block.getLocation ().getLine ());
	    if (!block.getType ().is<Void> ()) {
		var = Tree::varDecl (block.getLocation (), varName, generateType (block.getType ()));
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);
	    }
	    
	    enterBlock (block.getLocation ().toString ());
	    
	    TreeStmtList list = TreeStmtList::init ();	    
	    Generator last (Generator::empty ());
	    for (auto & it : block.getContent ()) {
		if (!last.isEmpty ()) list.append (generateValue (last));
		last = it;
	    }

	    if (!block.getType ().is<Void> ()) {
		auto value =  castTo (block.getType (), last);		
		list.append (value.getList ());
		value = value.getValue ();
		list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), block.getLocation (), var, value));
				
		auto binding = quitBlock (block.getLocation (), list.toTree (), block.getLocation ().toString ());
		auto ret = Tree::compound (block.getLocation (),
				       var, 
				       binding.bind_expr);
		return ret;
	    } else {
		if (!last.isEmpty ())
		    list.append (generateValue (last));
		auto binding = quitBlock (block.getLocation (), list.toTree (), block.getLocation ().toString ());
		return binding.bind_expr;
	    }    
	}

	Tree Visitor::generateLeftBlock (const Block & block) {
	    // A left block does not enter a new block
	    // Its return value is directly the last value
	    TreeStmtList list (TreeStmtList::init ());
	    Generator last (Generator::empty ());
	    
	    for (auto & it : block.getContent ()) {
		if (!last.isEmpty ()) list.append (generateValue (last));
		last = it;
	    }

	    return Tree::compound (
		block.getLocation (),
		generateValue (last),
		list.toTree ()
	    );
	}	

	Tree Visitor::generateSet (const Set & set) {
	    TreeStmtList list = TreeStmtList::init ();
	    Generator last (Generator::empty ());
	    Tree var (Tree::empty ());
	    if (!set.getType ().is<Void> ()) {
		var = Tree::varDecl (set.getLocation (), "_", generateType (set.getType ()));
	    }

	    for (auto & it : set.getContent ()) {
		if (!last.isEmpty ()) list.append (generateValue (last));
		last = it;
	    }

	    if (!set.getType ().is<Void> ()) {
		auto value = castTo (set.getType (), last);
		list.append (value.getList ());
		value = value.getValue ();
		
		list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), set.getLocation (), var, value));
		auto binding = list.toTree ();
		return Tree::compound (set.getLocation (),
				       var, 
				       binding);
	    } else {
		if (!last.isEmpty ())
		    list.append (generateValue (last));
		
		return list.toTree ();
	    }    
	}	
		
	Tree Visitor::generateFixed (const Fixed & fixed) {
	    auto type = generateType (fixed.getType ());	    
	    return Tree::buildIntCst (fixed.getLocation (), fixed.getUI ().u, type);
	}

	Tree Visitor::generateBool (const BoolValue & b) {
	    auto type = generateType (b.getType ());
	    return Tree::buildIntCst (b.getLocation (), (ulong) b.getValue (), type);
	}

	Tree Visitor::generateFloat (const FloatValue & f) {	    
	    auto type = generateType (f.getType ());
	    if (f.isStr ())
		return Tree::buildFloatCst (f.getLocation (), f.getValue (), type);
	    else {
		if (f.getType ().to <Float> ().getSize () == 32) {
		    return Tree::buildFloatCst (f.getLocation (), f.getValueFloat (), type);
		} else
		    return Tree::buildFloatCst (f.getLocation (), f.getValueDouble (), type);
	    }
	}

	Tree Visitor::generateChar (const CharValue & c) {
	    auto type = generateType (c.getType ());
	    return Tree::buildCharCst (c.getLocation (), c.getValue (), type);
	}
	
	Tree Visitor::generateAffect (const Affect & aff) {
	    auto leftType = aff.getWho ().to <Value> ().getType ();
	    // An affectation cannot generate ref copy, (or it is a construction)
	    if (!aff.isConstruction ()) {
		leftType = Type::init (leftType.to<Type> (), leftType.to <Type> ().isMutable (), false);
	    }
	    
	    auto left = castTo (leftType, aff.getWho ());	    
	    auto right = castTo (leftType, aff.getValue ()); 

	    TreeStmtList list = TreeStmtList::init ();
	    list.append (left.getList ());
	    list.append (right.getList ());
	    
	    auto lvalue =  left.getValue ();
	    auto rvalue =  right.getValue ();
	    
	    auto value = Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), aff.getLocation (), lvalue, rvalue);
	    auto ret = Tree::compound (aff.getLocation (), value, list.toTree ());
	    return ret;
	}

	Tree Visitor::generateBinaryInt (const BinaryInt & bin) {
	    auto left = generateValue (bin.getLeft ());
	    auto right = generateValue (bin.getRight ());

	    TreeStmtList list = TreeStmtList::init ();
	    list.append (left.getList ());
	    list.append (right.getList ());

	    tree_code code = LSHIFT_EXPR; // Fake default affectation to avoid warning
	    switch (bin.getOperator ()) {
	    case Binary::Operator::LEFT_SHIFT : code = LSHIFT_EXPR; break;
	    case Binary::Operator::RIGHT_SHIFT : code = RSHIFT_EXPR; break;
	    case Binary::Operator::BIT_OR : code = BIT_IOR_EXPR; break;
	    case Binary::Operator::BIT_AND : code = BIT_AND_EXPR; break;
	    case Binary::Operator::BIT_XOR : code = BIT_XOR_EXPR; break;
	    case Binary::Operator::ADD : code = PLUS_EXPR; break;
	    case Binary::Operator::SUB : code = MINUS_EXPR; break;
	    case Binary::Operator::MUL : code = MULT_EXPR; break;
	    case Binary::Operator::DIV : code = TRUNC_DIV_EXPR; break;
	    case Binary::Operator::MODULO : code = TRUNC_MOD_EXPR; break;
	    case Binary::Operator::INF : code = LT_EXPR; break;
	    case Binary::Operator::SUP : code = GT_EXPR; break;
	    case Binary::Operator::INF_EQUAL : code = LE_EXPR; break;
	    case Binary::Operator::SUP_EQUAL : code = GE_EXPR; break;
	    case Binary::Operator::EQUAL : code = EQ_EXPR; break;
	    case Binary::Operator::NOT_EQUAL : code = NE_EXPR; break;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }

	    auto type = generateType (bin.getType ());
	    
	    auto lvalue = left.getValue ();
	    auto rvalue = right.getValue ();
	    
	    auto value = Tree::binary (bin.getLocation (), code, type, lvalue, rvalue);	    
	    auto ret = Tree::compound (bin.getLocation (), value, list.toTree ());
	    return ret;
	}

	Tree Visitor::generateBinaryBool (const BinaryBool & bin) {
	    auto left = generateValue (bin.getLeft ());
	    auto right = generateValue (bin.getRight ());
	    TreeStmtList list = TreeStmtList::init ();
	    	    
	    auto lvalue = left.getValue ();
	    auto rvalue = right.getValue ();
	    list.append (left.getList ());
	    
	    if (bin.getOperator () == Binary::Operator::AND) {
		Tree var = Tree::varDecl (bin.getLocation (), Ymir::format ("_bb(%)", bin.getLocation ().getLine ()), generateType (bin.getType ()));
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);

		TreeStmtList if_L (TreeStmtList::init ());
		if_L.append (right.getList ());
		if_L.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), bin.getLocation (), var, rvalue));

		TreeStmtList if_nL (TreeStmtList::init ());
		if_nL.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), bin.getLocation (), var, lvalue));
		
		list.append (Tree::conditional (bin.getLocation (), getCurrentContext (), lvalue, if_L.toTree (), if_nL.toTree ()));
		lvalue = var;
	    } else {
		list.append (right.getList ());
	    }

	    tree_code code = LSHIFT_EXPR; // Fake default affectation to avoid warning
	    switch (bin.getOperator ()) {
	    case Binary::Operator::AND : code = TRUTH_ANDIF_EXPR; break;
	    case Binary::Operator::OR : code = TRUTH_ORIF_EXPR; break;
	    case Binary::Operator::EQUAL : code = EQ_EXPR; break;
	    case Binary::Operator::NOT_EQUAL : code = NE_EXPR; break;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }
	    
	    auto type = generateType (bin.getType ());
	    auto value = Tree::binary (bin.getLocation (), code, type, lvalue, rvalue);
	    auto ret = Tree::compound (bin.getLocation (), value, list.toTree ());
	    return ret;
	}

	Tree Visitor::generateBinaryChar (const BinaryChar & bin) {
	    auto left = generateValue (bin.getLeft ());
	    auto right = generateValue (bin.getRight ());

	    TreeStmtList list = TreeStmtList::init ();
	    list.append (left.getList ());
	    list.append (right.getList ());
	    
	    tree_code code = PLUS_EXPR; // Fake default affectation to avoid warning
	    switch (bin.getOperator ()) {
	    case Binary::Operator::ADD : code = PLUS_EXPR; break;
	    case Binary::Operator::SUB : code = MINUS_EXPR; break;
	    case Binary::Operator::MUL : code = MULT_EXPR; break;
	    case Binary::Operator::DIV : code = RDIV_EXPR; break;
		
	    case Binary::Operator::INF : code = LT_EXPR; break;
	    case Binary::Operator::SUP : code = GT_EXPR; break;
	    case Binary::Operator::INF_EQUAL : code = LE_EXPR; break;
	    case Binary::Operator::SUP_EQUAL : code = GE_EXPR; break;
	    case Binary::Operator::EQUAL : code = EQ_EXPR; break;
	    case Binary::Operator::NOT_EQUAL : code = NE_EXPR; break;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }
	    
	    auto lvalue = left.getValue ();
	    auto rvalue = right.getValue ();
	    
	    auto type = generateType (bin.getType ());
	    auto value = Tree::binary (bin.getLocation (), code, type, lvalue, rvalue);
	    auto ret = Tree::compound (bin.getLocation (), value, list.toTree ());
	    return ret;	    
	}

	
	Tree Visitor::generateBinaryFloat (const BinaryFloat & bin) {
	    auto left = generateValue (bin.getLeft ());
	    auto right = generateValue (bin.getRight ());

	    TreeStmtList list = TreeStmtList::init ();
	    list.append (left.getList ());
	    list.append (right.getList ());
	    
	    tree_code code = PLUS_EXPR; // Fake default affectation to avoid warning
	    switch (bin.getOperator ()) {
	    case Binary::Operator::ADD : code = PLUS_EXPR; break;
	    case Binary::Operator::SUB : code = MINUS_EXPR; break;
	    case Binary::Operator::MUL : code = MULT_EXPR; break;
	    case Binary::Operator::DIV : code = RDIV_EXPR; break;
		
	    case Binary::Operator::INF : code = LT_EXPR; break;
	    case Binary::Operator::SUP : code = GT_EXPR; break;
	    case Binary::Operator::INF_EQUAL : code = LE_EXPR; break;
	    case Binary::Operator::SUP_EQUAL : code = GE_EXPR; break;
	    case Binary::Operator::EQUAL : code = EQ_EXPR; break;
	    case Binary::Operator::NOT_EQUAL : code = NE_EXPR; break;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }
	    
	    auto lvalue = left.getValue ();
	    auto rvalue = right.getValue ();
	    
	    auto type = generateType (bin.getType ());
	    auto value = Tree::binary (bin.getLocation (), code, type, lvalue, rvalue);
	    auto ret = Tree::compound (bin.getLocation (), value, list.toTree ());
	    return ret;	    
	}

	Tree Visitor::generateBinaryPtr (const BinaryPtr & bin) {
	    auto left = generateValue (bin.getLeft ());
	    auto right = generateValue (bin.getRight ());
	    
	    TreeStmtList list = TreeStmtList::init ();
	    list.append (left.getList ());
	    list.append (right.getList ());
	    bool isTest = false;
	    tree_code code = PLUS_EXPR; // Fake default affectation to avoid warning
	    switch (bin.getOperator ()) {
	    case Binary::Operator::ADD : code = POINTER_PLUS_EXPR; break;
	    case Binary::Operator::SUB : code = POINTER_DIFF_EXPR; break;
	    case Binary::Operator::IS : {isTest = true; code = EQ_EXPR;} break;
	    case Binary::Operator::NOT_IS : {isTest = true; code = NE_EXPR;} break;		
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }

	    auto lvalue = left.getValue ();
	    auto rvalue = right.getValue ();

	    if (!isTest) {
		auto type = generateType (bin.getType ());
		auto value = Tree::binaryPtr (bin.getLocation (), code, type, lvalue, rvalue);
		auto ret = Tree::compound (bin.getLocation (), value, list.toTree ());
		return ret;
	    } else {
		auto type = generateType (bin.getType ());
		auto value = Tree::binaryPtrTest (bin.getLocation (), code, type, lvalue, rvalue);
		auto ret = Tree::compound (bin.getLocation (), value, list.toTree ());
		return ret;
	    }
	}
	
	generic::Tree Visitor::generateUnaryInt (const UnaryInt & un) {
	    auto value = generateValue (un.getOperand ());
	    auto type = generateType (un.getType ());
	    TreeStmtList list = TreeStmtList::init ();
	    list.append (value.getList ());

	    value = value.getValue ();

	    tree_code code = NEGATE_EXPR;
	    switch (un.getOperator ()) {
	    case Unary::Operator::MINUS : code = NEGATE_EXPR; break;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }
	    
	    return Tree::compound (
		un.getLocation (),
		Tree::unary (un.getLocation (), code, type, value),
		list.toTree ()
	    );
	}

	generic::Tree Visitor::generateUnaryFloat (const UnaryFloat & un) {
	    auto value = generateValue (un.getOperand ());
	    auto type = generateType (un.getType ());
	    TreeStmtList list = TreeStmtList::init ();
	    list.append (value.getList ());

	    value = value.getValue ();

	    tree_code code = NEGATE_EXPR;
	    switch (un.getOperator ()) {
	    case Unary::Operator::MINUS : code = NEGATE_EXPR; break;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }
	    
	    return Tree::compound (
		un.getLocation (),
		Tree::unary (un.getLocation (), code, type, value),
		list.toTree ()
	    );
	}

	generic::Tree Visitor::generateUnaryBool (const UnaryBool & un) {
	    if (un.getOperator () == Unary::Operator::NOT) {
		auto value = generateValue (un.getOperand ());
		auto type = generateType (un.getType ());
		TreeStmtList list = TreeStmtList::init ();
		list.append (value.getList ());
		value = value.getValue ();

		return Tree::compound (
		    un.getLocation (),
		    Tree::binary (un.getLocation (), BIT_XOR_EXPR, type, value, Tree::buildBoolCst (un.getLocation (), true)),
		    list.toTree ()
		);
	    } else {
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
		return Tree::empty ();
	    }
	}


	generic::Tree Visitor::generateUnaryPointer (const UnaryPointer & un) {
	    if (un.getOperator () == Unary::Operator::UNREF) {
		auto value = generateValue (un.getOperand ());
		TreeStmtList list = TreeStmtList::init ();
		list.append (value.getList ());
		value = value.getValue ();

		auto type = generateType (un.getType ());
		return Tree::compound (
		    un.getLocation (),
		    value.buildPointerUnref (type, 0),
		    list.toTree ()
		);
	    } else {
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
		return Tree::empty ();
	    }
	}
	
	generic::Tree Visitor::generateVarRef (const VarRef & var) {
	    return this-> getDeclarator (var.getRefId ());
	}	

	generic::Tree Visitor::generateVarDecl (const VarDecl & var) {
	    if (var.getVarType ().is<LambdaType> ()) return Tree::empty ();
	    
	    auto type = generateType (var.getVarType ());
	    auto name = var.getName ();

	    auto decl = Tree::varDecl (var.getLocation (), name, type);
	    if (!var.getVarValue ().isEmpty ()) {
		auto value = castTo (var.getVarType (), var.getVarValue ());
		decl.setDeclInitial (value);
	    } 

	    decl.setDeclContext (getCurrentContext ());
	    stackVarDeclChain.back ().append (decl);
	    
	    insertDeclarator (var.getUniqId (), decl);
	    return Tree::declExpr (var.getLocation (), decl);
	}

	generic::Tree Visitor::generateReferencer (const Referencer & ref) {
	    auto inner = castTo (ref.getType (), ref.getWho ());
	    if (ref.getWho ().to <Value> ().getType ().to <Type> ().isRef ())
	    	return inner;

	    auto type = generateType (ref.getType ());
	    return Tree::buildAddress (ref.getLocation (), inner, type);
	}	

	generic::Tree Visitor::generateAddresser (const Addresser & addr) {
	    auto inner = castTo (addr.getType (), addr.getWho ());
	    auto type = generateType (addr.getType ());

	    if (addr.getType ().is <FuncPtr> ()) // If it is a func pointer, we already get its address
		return inner;
	    else if (addr.getWho ().to <Value> ().getType ().to <Type> ().isRef ())
		return Tree::buildAddress (addr.getLocation (), inner.toDirect (), type);
	    else
		return Tree::buildAddress (addr.getLocation (), inner, type);
	}
	
	generic::Tree Visitor::generateConditional (const Conditional & cond) {
	    Tree var (Tree::empty ());
	    if (!cond.getType ().is<Void> ()) {
		var = Tree::varDecl (cond.getLocation (), Ymir::format ("_c(%)", cond.getLocation ().getLine ()), generateType (cond.getType ()));
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);
	    }

	    auto test = generateValue (cond.getTest ());
	    enterBlock (cond.getLocation ().toString ());
	    
	    Tree content (Tree::empty ());
	    if (!var.isEmpty () && !cond.getContent ().to <Value> ().getType ().is <Void> ()) {
	    	// The type can be different if this content is a returner
	    	TreeStmtList list = TreeStmtList::init ();
	    	content = castTo (cond.getType (), cond.getContent ());
	    	list.append (content.getList ());
	    	auto value = content.getValue ();
		
	    	list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), cond.getLocation (), var, value));
	    	content = list.toTree ();
	    } else content = generateValue (cond.getContent ());
	    
	    auto elsePart = Tree::empty ();
	    if (!cond.getElse ().isEmpty ()) {
	    	if (!var.isEmpty () && !cond.getElse ().to <Value> ().getType ().is<Void> ()) {
	    	    // The type can be different if this.else content is a returner
	    	    TreeStmtList list = TreeStmtList::init ();
	    	    elsePart = castTo (cond.getType (), cond.getElse ());
	    	    list.append (elsePart.getList ());
	    	    auto value = elsePart.getValue ();
		
	    	    list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), cond.getLocation (), var, value));
	    	    elsePart = list.toTree ();
	    	} else elsePart = generateValue (cond.getElse ());
	    }

	    TreeStmtList all = TreeStmtList::init ();	    
	    all.append (Tree::conditional (cond.getLocation (), getCurrentContext (), test, content, elsePart));
	    
	    auto binding = quitBlock (cond.getLocation (), all.toTree (), cond.getLocation ().toString ());	    
	    if (!var.isEmpty ()) {
		return Tree::compound (cond.getLocation (),
				       var,
				       binding.bind_expr
		    );
	    } else {
		return binding.bind_expr;
	    }
	}

	generic::Tree Visitor::generateLoop (const Loop & loop) {
	    Tree var (Tree::empty ());
	    Tree test (Tree::empty ());
	    if (!loop.getTest ().isEmpty ())
		test = generateValue (loop.getTest ());
	    
	    if (!loop.getType ().is<Void> ()) {
		var = Tree::varDecl (loop.getLocation (), Ymir::format ("_l(%)", loop.getLocation ().getLine ()), generateType (loop.getType ()));
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);
	    }
	    
	    enterBlock (loop.getLocation ().toString ());

	    auto end_label = Tree::makeLabel (loop.getLocation (), getCurrentContext (), "end");
	    enterLoop (end_label, var);
	    Tree content (Tree::empty ());

	    // The last value is not used when we have a loop {} expression
	    // So, we have to verify if the loop has a test 
	    if (!var.isEmpty () && !loop.getContent ().to <Value> ().isBreaker () && !test.isEmpty ()) {
		TreeStmtList list = TreeStmtList::init ();
		content = castTo (loop.getType (), loop.getContent ());
		list.append (content.getList ());
		auto value = content.getValue ();

		list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), loop.getLocation (), var, value));		
		content = list.toTree ();
	    } else content = generateValue (loop.getContent ());
	    quitLoop ();
	    
	    TreeStmtList all = TreeStmtList::init ();
	    auto begin_label = Tree::makeLabel (loop.getLocation (), getCurrentContext (), "begin");
	    auto test_label = Tree::makeLabel (loop.getLocation (), getCurrentContext (), "test");
	    
	    if (!loop.isDo () && !test.isEmpty ())
		all.append (Tree::gotoExpr (loop.getLocation (), test_label));
	    
	    all.append (Tree::labelExpr (loop.getLocation (), begin_label));
	    all.append (content);
	    if (!test.isEmpty ()) {
		all.append (Tree::labelExpr (loop.getLocation (), test_label));
		all.append (Tree::condExpr (loop.getLocation (),
					    test,
					    Tree::gotoExpr (loop.getLocation (), begin_label),
					    Tree::gotoExpr (loop.getLocation (), end_label)
		));
	    } else {
		all.append  (Tree::gotoExpr (loop.getLocation (), begin_label));
	    }

	    all.append (Tree::labelExpr (loop.getLocation (), end_label));
	    
	    auto binding = quitBlock (loop.getLocation (), all.toTree (), loop.getLocation ().toString ());	    
	    return Tree::compound (loop.getLocation (),
				   var, 
				   binding.bind_expr
		);
	}

	generic::Tree Visitor::generateBreak (const Break & br) {
	    TreeStmtList list = TreeStmtList::init ();
	    if (!br.getValue ().to <Value> ().getType ().is<Void> ()) {
	    	auto value = generateValue (br.getValue ());		 // Loop will never be lvalue, and therefore cannot return a ref
	    	list.append (
	    	    Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), br.getLocation (),
	    			  this-> _loopVars.back (),
	    			  value)
	    	);
	    }

	    if (this-> exceptionDeclChain.back ().back ().label.isEmpty ()) {
		// If we are in a loop, we pushed a exceptionDeclChain thing, but an empty one
		list.append (Tree::gotoExpr (br.getLocation (), this-> _loopLabels.back ()));
	    } else { // or a catcher put a real exception and we have to take it into account
		auto exc_break = this-> exceptionDeclChain.back ().back ();
		auto test_val = Tree::buildIntCst (br.getLocation (), ReturnWithinCatch::BREAK, Tree::intType (8, false));
		list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), br.getLocation (), exc_break.test, test_val));
		list.append (this-> popLastException (br.getLocation ()));
		list.append (Tree::gotoExpr (br.getLocation (), exc_break.label));
	    }
	    return list.toTree ();
	}

	generic::Tree Visitor::generateBreak (const lexing::Word & loc) {
	    if (this-> exceptionDeclChain.back ().back ().label.isEmpty ()) {
		// If we are in a loop, we pushed a exceptionDeclChain thing, but an empty one
		return Tree::gotoExpr (loc, this-> _loopLabels.back ());
	    } else {
		TreeStmtList list = TreeStmtList::init ();
		auto exc_break = this-> exceptionDeclChain.back ().back ();
		auto test_val = Tree::buildIntCst (loc, ReturnWithinCatch::BREAK, Tree::intType (8, false));
		list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), loc, exc_break.test, test_val));
		list.append (this-> popLastException (loc));
		list.append (Tree::gotoExpr (loc, exc_break.label));
		return list.toTree ();	    
	    }
	}
	
	generic::Tree Visitor::generateReturn (const Return & ret) {
	    TreeStmtList list = TreeStmtList::init ();	    
	    if (!ret.getValue ().isEmpty ()) {
		auto value = castTo (ret.getFunType (), ret.getValue ());
		auto fr = getCurrentContext ();
		auto resultDecl = fr.getResultDecl ();

		if (ret.getFunType ().is<Void> ()) {
		    list.append (value);
		    auto exc_return = this-> getLastCatcherPosition ();
		    if (exc_return.test.isEmpty ()) 
			list.append (Tree::returnStmt (ret.getLocation ()));
		    else {
			auto test_val = Tree::buildIntCst (ret.getLocation (), ReturnWithinCatch::RETURN, Tree::intType (8, false));
			list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), ret.getLocation (), exc_return.test, test_val));
			list.append (this-> popLastException (ret.getLocation ()));
			list.append (Tree::gotoExpr (ret.getLocation (), exc_return.label));
		    }
		} else {
		    list.append (value.getList ());
		    value = value.getValue ();
		    auto exc_return = this-> getLastCatcherPosition ();
		    if (exc_return.test.isEmpty ()) {
			list.append (Tree::returnStmt (ret.getLocation (), resultDecl, value));
		    } else {
			auto test_val = Tree::buildIntCst (ret.getLocation (), ReturnWithinCatch::RETURN, Tree::intType (8, false));			
			list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), ret.getLocation (), exc_return.test, test_val));
			list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), ret.getLocation (), exc_return.var, value));
			list.append (this-> popLastException (ret.getLocation ()));
			list.append (Tree::gotoExpr (ret.getLocation (), exc_return.label));
		    }
		}
	    } else {
		auto exc_return = this-> getLastCatcherPosition ();
		if (exc_return.test.isEmpty ()) {		    
		    list.append (Tree::returnStmt (ret.getLocation ()));
		} else {
		    auto test_val = Tree::buildIntCst (ret.getLocation (), ReturnWithinCatch::RETURN, Tree::intType (8, false));
		    list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), ret.getLocation (), exc_return.test, test_val));
		    list.append (this-> popLastException (ret.getLocation ()));
		    list.append (Tree::gotoExpr (ret.getLocation (), exc_return.label));
		}
	    }
	    
	    return list.toTree ();
	}

	generic::Tree Visitor::generateReturn (const lexing::Word & location, generic::Tree value) {
	    TreeStmtList list = TreeStmtList::init ();
	    if (!value.isEmpty ()) {
		auto exc_return = this-> getLastCatcherPosition ();
		if (exc_return.test.isEmpty ()) {
		    auto fr = getCurrentContext ();
		    auto resultDecl = fr.getResultDecl ();
		    
		    return Tree::returnStmt (location, resultDecl, value);
		} else {
		    auto test_val = Tree::buildIntCst (location, ReturnWithinCatch::RETURN, Tree::intType (8, false));
		    list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), location, exc_return.test, test_val));
		    list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), location, exc_return.var, value));
		    list.append (this-> popLastException (location));
		    list.append (Tree::gotoExpr (location, exc_return.label));
		}
	    } else {
		auto exc_return = this-> getLastCatcherPosition ();
		if (exc_return.test.isEmpty ()) {
		    return Tree::returnStmt (location);
		} else {
		    auto test_val = Tree::buildIntCst (location, ReturnWithinCatch::RETURN, Tree::intType (8, false));
		    list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), location, exc_return.test, test_val));
		    list.append (this-> popLastException (location));
		    list.append (Tree::gotoExpr (location, exc_return.label));
		}
	    }
	    return list.toTree ();
	}

	Visitor::ReturnWithinCatch Visitor::getLastCatcherPosition () {
	    for (auto i : Ymir::r ((int) (this-> exceptionDeclChain.back ().size () - 1), -1)) {
		if (!this-> exceptionDeclChain.back () [i].test.isEmpty ())
		    return this-> exceptionDeclChain.back () [i];
	    }
	    return ReturnWithinCatch {Tree::empty (), Tree::empty (), Tree::empty (), Tree::empty ()};
	}
	
	generic::Tree Visitor::generateRangeValue (const RangeValue & rng) {
	    auto type = generateType (rng.getType ());
	    std::vector <Tree> params = {
		generateValue (rng.getLeft ()),
		generateValue (rng.getRight ()),
		generateValue (rng.getStep ()),
		generateValue (rng.getIsFull ())
	    };
	    return Tree::constructField (rng.getLocation (), type, {Range::FST_NAME, Range::SCD_NAME, Range::STEP_NAME, Range::FULL_NAME}, params);
	}

	generic::Tree Visitor::generateSliceValue (const SliceValue & slc) {
	    auto type = generateType (slc.getType ());
	    auto ptrValue = generateValue (slc.getPtr ());
	    
	    std::vector <Tree> params = {
		generateValue (slc.getLen ()),
		ptrValue
	    };	    
	    
	    return Tree::constructField (slc.getLocation (), type, {Slice::LEN_NAME, Slice::PTR_NAME}, params);
	}

	generic::Tree Visitor::generateDelegateValue (const DelegateValue & dlg) {
	    auto type = generateType (dlg.getType ());
	    std::vector <Tree> params = {
		castTo (dlg.getClosureType (), dlg.getClosure ()),
		generateValue (dlg.getFuncPtr ())
	    };
	    
	    return Tree::constructField (dlg.getLocation (), type, {}, params);
	}
	
	generic::Tree Visitor::generateCast (const Cast & cast) {
	    auto type = generateType (cast.getType ());
	    auto who = generateValue (cast.getWho ());	    
	    return Tree::castTo (cast.getLocation (), type, who);
	}

	generic::Tree Visitor::generateAtomicLocker (const AtomicLocker & lock) {
	    auto fn = global::CoreNames::get (ATOMIC_LOCK);
	    if (lock.isMonitor ()) fn = global::CoreNames::get (ATOMIC_MONITOR_LOCK);
	    
	    auto inner = generateValue (lock.getWho ());
	    return Tree::buildCall (
		lock.getLocation (),
		Tree::voidType (),
		fn,
		{inner}
		);
	}

	generic::Tree Visitor::generateAtomicUnlocker (const AtomicUnlocker & lock) {
	    auto fn = global::CoreNames::get (ATOMIC_UNLOCK);
	    if (lock.isMonitor ()) fn = global::CoreNames::get (ATOMIC_MONITOR_UNLOCK);
	    
	    auto inner = generateValue (lock.getWho ());
	    return Tree::buildCall (
		lock.getLocation (),
		Tree::voidType (),
		fn,
		{inner}
		);
	}
	
	generic::Tree Visitor::generateArrayAlloc (const ArrayAlloc & alloc) {
	    auto value = generateValue (alloc.getDefaultValue ());
	    auto size = generateValue (alloc.getInnerTypeSize ());
	    auto valPtr = value;
	    if (!alloc.getDefaultValue ().to <Value> ().getType ().to <Type> ().isRef ()) {
	    	auto type = generateType (alloc.getDefaultValue ().to <Value> ().getType ());
	    	valPtr = Tree::buildAddress (alloc.getLocation (), value, Tree::pointerType (type));
	    }
	    
	    if (alloc.isDynamic ()) {
		auto len = generateValue (alloc.getDynLen ());
		return Tree::buildCall (
		    alloc.getLocation (),
		    generateType (alloc.getType ()),
		    global::CoreNames::get (ARRAY_ALLOC),
		    {valPtr, size, len}		
		);
	    } else {
		std::vector <Tree> params;
		TreeStmtList list = TreeStmtList::init ();
		list.append (value.getList ());
		for (auto it ATTRIBUTE_UNUSED : Ymir::r (0, alloc.getStaticLen ())) {
		    params.push_back (value.getValue ());
		}
		auto type = generateType (alloc.getType ());
		auto index = Tree::constructIndexed (alloc.getLocation (), type, params);
		if (!list.isEmpty ()) {		
		    return Tree::compound (
			alloc.getLocation (),
			index,
			list.toTree ()
			);
		} else {
		    return index;
		}
	    }
	}	
	
	generic::Tree Visitor::generateNullValue (const NullValue & nl) {
	    return Tree::buildPtrCst (nl.getLocation (), 0);
	}

	generic::Tree Visitor::generateUniqValue (const UniqValue & uniq) {
	    auto ref = getDeclaratorOrEmpty (uniq.getRefId ());
	    if (!ref.isEmpty ()) return ref;
	    else {
		auto type = generateType (uniq.getValue ().to <Value> ().getType ());
		auto name = Ymir::format ("uniq_%", uniq.getRefId ());
		auto decl = Tree::varDecl (uniq.getLocation (), name, type);
		decl.setDeclInitial (castTo (uniq.getValue ().to <Value> ().getType (), uniq.getValue ()));
		decl.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (decl);
	    
		insertDeclarator (uniq.getRefId (), decl);

		TreeStmtList list = TreeStmtList::init ();
		list.append (Tree::declExpr (uniq.getLocation (), decl));
		list.append (decl);

		return Tree::compound (
		    uniq.getLocation (),
		    decl,
		    list.toTree ()
		);
	    }
	}

	generic::Tree Visitor::generateThrow (const Throw & thr) {
	    auto value = castTo (thr.getValue ().to <Value> ().getType (), thr.getValue ());
	    auto info = generateValue (thr.getTypeInfo ());
	    auto file = thr.getLocation ().getFilename ();
	    auto lit = Tree::buildStringLiteral (thr.getLocation (), file.c_str (), file.length () + 1, 8);
	    auto context = getCurrentContext ().funcDeclName ();
	    auto func = Tree::buildStringLiteral (thr.getLocation (), context.c_str (), context.length () + 1, 8);
	    
	    auto line = Tree::buildIntCst (thr.getLocation (), (ulong) thr.getLocation ().getLine (), Tree::intType (32, false));

	    return Tree::buildCall (
		thr.getLocation (),
		Tree::voidType (),
		global::CoreNames::get (THROW),
		{lit, func, line, info, value}
	    );
	}

	generic::Tree Visitor::generateThrowBlock (const ThrowBlock & bl) {
	    auto lbl = this-> _throwLabels.back ().find (bl.getName ());
	    
	    if (lbl == this-> _throwLabels.back ().end ()) {
		auto endLabel = Tree::makeLabel (bl.getLocation (), getCurrentContext (), "end_throw");
		auto beginLabel = Tree::makeLabel (bl.getLocation (), getCurrentContext (), "begin_throw");
		auto doLabel = Tree::makeLabel (bl.getLocation (), getCurrentContext (), "do_throw");
		
		enterBlock (bl.getLocation ().toString ());
		TreeStmtList list (TreeStmtList::init ());
		list.append (Tree::labelExpr (bl.getLocation (), beginLabel));
		list.append (Tree::gotoExpr (bl.getLocation (), endLabel));
		list.append (Tree::labelExpr (bl.getLocation (), doLabel));
		list.append (generateValue (bl.getContent ()));
		list.append (Tree::labelExpr (bl.getLocation (), endLabel));
		auto binding = quitBlock (bl.getLocation (), list.toTree (), bl.getLocation ().toString ());
		TreeStmtList all (TreeStmtList::init ());
		all.append (binding.bind_expr);
		all.append (Tree::gotoExpr (bl.getLocation (), doLabel));
		this-> _throwLabels.back ().emplace (bl.getName (), doLabel);
		
		return all.toTree ();
	    }
	    
	    return Tree::gotoExpr (bl.getLocation (), lbl-> second);
	}

	generic::Tree Visitor::generateOptionValue (const OptionValue & val) {
	    std::vector <std::string> names;
	    std::vector <Tree> results;
	    names.push_back (Option::TYPE_FIELD);
	    if (val.isSuccess () && !val.getContent ().to <Value> ().getType ().is <Void> ()) names.push_back (Option::VALUE_FIELD);
	    else if (!val.isSuccess ()) names.push_back (Option::ERROR_FIELD);

	    results.push_back (Tree::buildBoolCst (val.getLocation (), val.isSuccess ()));
	    auto value = generateValue (val.getContent ());
	    TreeStmtList list (TreeStmtList::init ());

	    list.append (value.getList ());
	    value = value.getValue ();

	    if (!val.getContent ().to <Value> ().getType ().is <Void> ())
		results.push_back (value);
	    else
		list.append (value);
	    
	    auto type = generateType (val.getType ());
	    return Tree::compound (val.getLocation (),
				   Tree::constructField (val.getLocation (), type, names, results),
				   list.toTree ());
	}
	
	generic::Tree Visitor::generateExitScope (const ExitScope & scope) {
	    auto r_jmp = Tree::varDecl (scope.getLocation (), "#buf", generateType (scope.getJmpbufType ()));
	    r_jmp.setDeclContext (getCurrentContext ());
	    stackVarDeclChain.back ().append (r_jmp);
	    	    
	    auto i_type = Integer::init (scope.getLocation (), 32, false);	    
	    auto r_res = Tree::varDecl (scope.getLocation (), "#ref", generateType (i_type));
	    r_res.setDeclContext (getCurrentContext ());
	    stackVarDeclChain.back ().append (r_res);

	    auto return_value = Tree::empty ();
	    auto test_return_value = Tree::varDecl (scope.getLocation (), "#ret?", Tree::intType (8, false));
	    test_return_value.setDeclContext (getCurrentContext ());
	    stackVarDeclChain.back ().append (test_return_value);
	    
	    if (!getCurrentContext ().getResultDecl ().getType ().isVoidType ()) {
		return_value = Tree::varDecl (scope.getLocation (), "#ret_val", getCurrentContext ().getResultDecl ().getType ());
		return_value.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (return_value);
	    }
	    
	    Tree var (Tree::empty ());
	    if (!scope.getWho ().to <Value> ().getType ().is <Void> ()) {
		var = Tree::varDecl (scope.getLocation (), Ymir::format ("exit_%", scope.getWho().getLocation().getLine ()), generateType (scope.getWho ().to <Value> ().getType ()));
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);
	    }

	    TreeStmtList list (TreeStmtList::init ());
	    enterBlock (scope.getLocation ().toString ());
	    
	    auto exit_label = Tree::makeLabel (scope.getLocation (), getCurrentContext (), "exit");
	    ReturnWithinCatch exc_return = {exit_label, test_return_value, return_value, r_jmp};
	    this-> exceptionDeclChain.back ().push_back (exc_return);
	    
	    list.append (r_jmp);
	    list.append (r_res);
	    list.append (test_return_value);

	    list.append (
	    	Tree::affect (
		    this-> stackVarDeclChain.back (), this-> getCurrentContext (),
	    	    scope.getLocation (),
	    	    r_res,
	    	    Tree::buildCall (scope.getLocation (), generateType (i_type), global::CoreNames::get (SET_JMP),  {Tree::buildAddress (scope.getLocation (), r_jmp, Tree::pointerType (Tree::voidType ()))})
	    	)
	    ); // res = setjmp (buf);

	    list.append (
		Tree::affect (this-> stackVarDeclChain.back (),
			      this-> getCurrentContext (),
			      scope.getLocation (),
			      test_return_value,
			      Tree::buildIntCst (scope.getLocation (),
						 ReturnWithinCatch::NONE,
						 Tree::intType (8, false)
				  )
		    )
		); // No return by default
	    
	    auto left_value = generateValue (scope.getWho ());	    
	    if (!scope.getWho ().to <Value> ().getType ().is <Void> ()) { // If the content block has a value
		TreeStmtList list (TreeStmtList::init ());		
		list.append (left_value.getList ());
		list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), scope.getLocation (), var, left_value.getValue ()));
		left_value = list.toTree ();
	    }

	    this-> exceptionDeclChain.back ().pop_back ();
	    
	    /**
	     * Exit scope
	     */
	    TreeStmtList exitList = TreeStmtList::init ();	    
	    exitList.append (Tree::labelExpr (scope.getLocation (), exit_label));
	    
	    for (auto & it : scope.getExit ()) {
	    	exitList.append (generateValue (it));
	    }
	    
	    /** Try scope */
	    TreeStmtList left_part (TreeStmtList::init ());
	    left_part.append (left_value);
	    left_part.append (Tree::buildCall (
				  scope.getLocation (),
				  Tree::boolType (),
				  global::CoreNames::get (EXCEPT_POP),
				  {Tree::buildAddress (scope.getLocation (), exc_return.jmp_var, Tree::pointerType (Tree::voidType ()))}
				  )
		);
	    
	    left_part.append (Tree::gotoExpr (scope.getLocation (), exit_label));
	    
	    /** Failure scope */
	    TreeStmtList right_part (TreeStmtList::init ());
	    for (auto & it : scope.getFailure ()) {
		right_part.append (generateValue (it));
	    }
	    
	    right_part.append (generateCatching (scope, var, exc_return)); // Affect the value of the block on failure	    
	    right_part.append (Tree::gotoExpr (scope.getLocation (), exit_label)); // Goto exit scope in all cases

	    
	    auto right = right_part.toTree ();
	    auto left = left_part.toTree ();
	    
	    auto test = Tree::buildCall (
		scope.getLocation (),
		Tree::boolType (),
		global::CoreNames::get (EXCEPT_PUSH),
		{Tree::buildAddress (scope.getLocation (), r_jmp, Tree::pointerType (Tree::voidType ())), r_res}
	    );
	    
	    /// Conditional on the jump
	    auto cond = Tree::conditional (scope.getLocation (), getCurrentContext (), test, left, right); // Try catch expression -> exit
	    list.append (cond);

	    exitList.append (this-> generateEndOfExit (scope.getLocation (), test_return_value, return_value));
	    list.append (exitList.toTree ());
	    
	    auto binding = quitBlock (scope.getLocation (), list.toTree (), scope.getLocation ().toString ());
	    if (!scope.getWho ().to <Value> ().getType ().is <Void> ()) {
		return Tree::compound (scope.getLocation (), var, binding.bind_expr);
	    } else return binding.bind_expr;
	}

	generic::Tree Visitor::generateCatching (const ExitScope & scope, Tree varScope, ReturnWithinCatch exc_return) {
	    TreeStmtList all (TreeStmtList::init ());
	    TreeStmtList rethrow = TreeStmtList::init ();

	    enterBlock (scope.getLocation ().toString ());
	    auto test_val = Tree::buildIntCst (scope.getLocation (), ReturnWithinCatch::THROW, Tree::intType (8, false));
	    
	    // By default we go to the exit scope and tell it that nothing has been caught
	    rethrow.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), scope.getLocation (), exc_return.test, test_val));
	    rethrow.append (Tree::gotoExpr (scope.getLocation (), exc_return.label));
	    	    
	    auto & var = scope.getCatchingVar ();
	    auto & info = scope.getCatchingInfoType ();
	    auto & action = scope.getCatchingAction ();
	    if (!var.isEmpty ()) {		
		auto type = generateType (var.to <generator::VarDecl> ().getVarType ());
		auto innerVar = Tree::varDecl (var.getLocation (), var.to <generator::VarDecl> ().getName (), type);
		innerVar.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (innerVar);
		insertDeclarator (var.getUniqId (), innerVar);

		auto innerInfo = generateValue (info);
		auto call = Tree::buildCall (var.getLocation (), type, global::CoreNames::get (EXCEPT_GET_VALUE), {innerInfo});
		auto left = Tree::compound (var.getLocation (), innerVar, Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), scope.getLocation (), innerVar, call));
		auto nul = Tree::buildPtrCst (var.getLocation (), 0);
		auto test = Tree::binaryPtrTest (var.getLocation (), NE_EXPR, Tree::boolType (), left, nul);
		
		TreeStmtList list (TreeStmtList::init ());
		if (!action.to <Value> ().getType ().is <Void> ()) {
		    auto act = castTo (action.to <Value> ().getType (), action);
		    list.append (act.getList ());
		    act = act.getValue ();
		    list.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), scope.getLocation (), varScope, act));
		} else
		    list.append (generateValue (action));
		
		all.append (Tree::conditional (scope.getLocation (), getCurrentContext (), test, list.toTree (), rethrow.toTree ()));
	    } else {
		all.append (rethrow.toTree ());
	    }
	    
	    auto binding = quitBlock (scope.getLocation (), all.toTree (), scope.getLocation ().toString ());	    
	    return binding.bind_expr;
	}

	generic::Tree Visitor::generateEndOfExit (const lexing::Word & loc, generic::Tree test, generic::Tree ret_value) {
	    auto type = Tree::intType (8, false);
	    auto is_return = Tree::binary (loc, EQ_EXPR, type, test, Tree::buildIntCst (loc, ReturnWithinCatch::RETURN, type));
	    auto is_break = Tree::binary (loc, EQ_EXPR, type, test, Tree::buildIntCst (loc, ReturnWithinCatch::BREAK, type));
	    auto is_none = Tree::binary (loc, EQ_EXPR, type, test, Tree::buildIntCst (loc, ReturnWithinCatch::NONE, type));

	    TreeStmtList list = TreeStmtList::init ();
	    auto rethrow = Tree::buildCall (loc, Tree::voidType (), global::CoreNames::get (RETHROW), {});

	    if (this-> _loopLabels.size () != 0) { // Can only break inside loops
		/**
		 * if (test == NONE) {} else if (test == RETURN) { return ; } else if (test == BREAK) { break; } else { rethrow ; }
		 */
		auto fst = Tree::conditional (loc, getCurrentContext (), is_break, this-> generateBreak (loc), rethrow);
		auto scd = Tree::conditional (loc, getCurrentContext (), is_return, this-> generateReturn (loc, ret_value), fst);
		list.append (Tree::conditional (loc, getCurrentContext (), is_none, TreeStmtList::init ().toTree (), scd));
		return list.toTree ();
	    } else {
		auto fst = Tree::conditional (loc, getCurrentContext (), is_return, this-> generateReturn (loc, ret_value), rethrow);					  
		list.append (Tree::conditional (loc, getCurrentContext (), is_none, TreeStmtList::init ().toTree (), fst));
		return list.toTree ();
	    } 
	}

	generic::Tree Visitor::popLastException (const lexing::Word & loc) {
	    for (auto i : Ymir::r ((int) (this-> exceptionDeclChain.back ().size () - 1), -1)) {
		if (!this-> exceptionDeclChain.back ()[i].jmp_var.isEmpty ()) {			
		    return Tree::buildCall (
			loc,
			Tree::boolType (),
			global::CoreNames::get (EXCEPT_POP),
			{Tree::buildAddress (loc, this-> exceptionDeclChain.back () [i].jmp_var, Tree::pointerType (Tree::voidType ()))}
			);
		}
	    }
	    
	    return Tree::empty ();	    
	}
	
	generic::Tree Visitor::generateSuccessScope (const SuccessScope & scope) {
	    auto test_return_value = Tree::varDecl (scope.getLocation (), "#ret?", Tree::intType (8, false));
	    test_return_value.setDeclContext (getCurrentContext ());
	    stackVarDeclChain.back ().append (test_return_value);
	    
	    Tree var (Tree::empty ());
	    if (!scope.getType ().is<Void> ()) {
		var = Tree::varDecl (scope.getLocation (), "_", generateType (scope.getType ()));
		var.setDeclContext (getCurrentContext ());		
		stackVarDeclChain.back ().append (var);
	    }

	    Tree return_value (Tree::empty ());
	    if (!getCurrentContext ().getResultDecl ().getType ().isVoidType ()) {
		return_value = Tree::varDecl (scope.getLocation (), "#ret_val", getCurrentContext ().getResultDecl ().getType ());
		return_value.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (return_value);
	    }
	    
	    TreeStmtList list (TreeStmtList::init ());
	    enterBlock (scope.getLocation ().toString ());
	    
	    list.append (
		Tree::affect (this-> stackVarDeclChain.back (),
			      this-> getCurrentContext (),
			      scope.getLocation (),
			      test_return_value,
			      Tree::buildIntCst (scope.getLocation (),
						 ReturnWithinCatch::NONE,
						 Tree::intType (8, false)
				  )
		    )
		); // No return by default

	   

	    auto exit_label = Tree::makeLabel (scope.getLocation (), getCurrentContext (), "exit");	    
	    this-> exceptionDeclChain.back ().push_back ({exit_label, test_return_value, return_value, Tree::empty ()});
	    auto content = generateValue (scope.getWho ());
	    this-> exceptionDeclChain.back ().pop_back ();
	    
	    /**
	     * Success scope
	     */
	    TreeStmtList exitList = TreeStmtList::init ();	    
	    exitList.append (Tree::labelExpr (scope.getLocation (), exit_label));
	    
	    for (auto & it : scope.getValues ()) {
	    	exitList.append (generateValue (it));
	    }
	    
	    if (!var.isEmpty ()) {
		TreeStmtList innerList (TreeStmtList::init ());
		innerList.append (content.getList ());
		innerList.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), scope.getLocation (), var, content.getValue ()));
		content = innerList.toTree ();
	    }
	   
	    list.append (content);
	    list.append (Tree::gotoExpr (scope.getLocation (), exit_label)); // We go to success scope
	    


	    exitList.append (this-> generateEndOfExit (scope.getLocation (), test_return_value, return_value));
	    list.append (exitList.toTree ());
	    
	    auto binding = quitBlock (scope.getLocation (), list.toTree (), scope.getLocation ().toString ());
	    if (!scope.getType ().is <Void> ()) {
		return Tree::compound (scope.getLocation (), var, binding.bind_expr);
	    } else return binding.bind_expr;
	}
	
	generic::Tree Visitor::generateArrayValue (const ArrayValue & val) {
	    auto type = generateType (val.getType ());
	    //auto inner = generateType (val.getType ().to <Array> ().getInners () [0]);
	    std::vector <Tree> params;
	    for (auto it : val.getContent ()) {
		auto value = castTo (val.getType ().to<Type> ().getInners ()[0], it);
		params.push_back (value);
	    }
	    return Tree::constructIndexed (val.getLocation (), type, params);
	}

	generic::Tree Visitor::generateTupleValue (const TupleValue & val) {
	    auto type = generateType (val.getType ());
	    std::vector <Tree> params;
	    for (auto it : Ymir::r (0, val.getContent ().size ())) {		    
		//auto inner = generateType (it.to<Value> ().getType ());
		auto value = castTo (val.getType().to<Type> ().getInners ()[it], val.getContent ()[it]);
		params.push_back (value);
	    }
	    return Tree::constructField (val.getLocation (), type, {}, params);
	}	

	generic::Tree Visitor::generateStringValue (const StringValue & str) {
	    auto inner = str.getType ().to <Array> ().getInners () [0];
	    auto len = str.getLen ();
	    auto size = inner.to<Char> ().getSize ();
	    const char * data = str.getValue ().data ();
	    return Tree::buildStringLiteral (str.getLocation (), data, len, size);
	}
	
	generic::Tree Visitor::generateFrameProto (const FrameProto & proto) {
	    std::vector <Tree> params;
	    for (auto & it : proto.getParameters ())
		params.push_back (generateType (it.to <ProtoVar> ().getType ()));


	    auto type = generateType (proto.getReturnType ());
	    auto name = Mangler::init ().mangleFrameProto (proto);
	    return Tree::buildFrameProto (proto.getLocation (), type, name, params);
	}

	generic::Tree Visitor::generateConstructorProto (const ConstructorProto & proto) {
	    std::vector <Tree> params;
	    params.push_back (generateType (proto.getReturnType ()));
	    for (auto & it : proto.getParameters ()) 
		params.push_back (generateType (it.to <ProtoVar> ().getType ()));

	    auto type = generateType (proto.getReturnType ());
	    auto name = Mangler::init ().mangleConstructorProto (proto);
	    return Tree::buildFrameProto (proto.getLocation (), type, name, params);
	}

	generic::Tree Visitor::generateTupleAccess (const TupleAccess & acc) {
	    auto elem = generateValue (acc.getTuple ());
	    auto field = Ymir::format ("_%", (int) acc.getIndex ());
	    return Tree::compound (
		acc.getLocation (),
		elem.getValue ().getField (field),
		elem.getList ()
	    );
	}

	generic::Tree Visitor::generateStructAccess (const StructAccess & acc) {
	    auto elem = generateValue (acc.getStruct ());
	    
	    // If the type is a class, we need to unref it to access its inner fields
	    // Same if it is a ref to a struct, or a ref to class
	    while (elem.getType ().isPointerType ())
		elem = elem.toDirect ();
	    
	    return Tree::compound (
		acc.getLocation (),
		elem.getValue ().getField (acc.getField ()),
		elem.getList ()
	    );
	}

	generic::Tree Visitor::generateVtableAccess (const VtableAccess & acc) {
	    auto elem = generateValue (acc.getClass ());
	    // If the type is a class, we need to unref it to access its inner fields
	    // Same if it is a ref to a struct, or a ref to class
	    
	    while (elem.getType ().isPointerType ())
		elem = elem.toDirect ();
	    
	    auto vtable = elem.getValue ().getField ("#_vtable");
	    auto type = generateType (acc.getType ());
	    return Tree::compound (
		acc.getLocation (),
		vtable.buildPointerUnref (type, acc.getField ()),
		elem.getList ()
	    );
	}

	generic::Tree Visitor::generateCall (const Call & cl) {
	    std::vector <Tree> results;
	    TreeStmtList pre = TreeStmtList::init ();

	    for (auto it : Ymir::r (0, cl.getTypes ().size ())) {
		auto val = castTo (cl.getTypes () [it], cl.getParameters () [it]);		
		results.push_back (val.getValue ());
		pre.append (val.getList ());
	    }

	    for (auto & it : cl.getAddParameters ()) {
		auto value = generateValue (it);
		pre.append (value.getList ());
		results.push_back (value.getValue ().promote ());
	    }

	    if (cl.getFrame ().to <Value> ().getType ().is <Delegate> ()) { // Delegate are {&closure, &fn}, so we call it this way : &fn (&closure, types...)
		auto fn = generateValue (cl.getFrame ());
		Tree var = Tree::varDecl (cl.getLocation (), "_", fn.getType ());
		var.setDeclContext (getCurrentContext ());
		
		stackVarDeclChain.back ().append (var);
		pre.append (Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), cl.getLocation (), var, fn));
		
		results.insert (results.begin (), var.getField (Ymir::format ("_%", 0)));		
		auto type = generateType (cl.getType ());
		return Tree::compound (
		    cl.getLocation (),
		    Tree::buildCall (cl.getLocation (), type, var.getField (Ymir::format ("_%", 1)), results),
		    pre.toTree ()
		);
	    } else {
		auto fn = generateValue (cl.getFrame ());	    
		auto type = generateType (cl.getType ());
		return Tree::compound (
		    cl.getLocation (), 
		    Tree::buildCall (cl.getLocation (), type, fn, results),
		    pre.toTree ()
		);
	    }
	}

	generic::Tree Visitor::generateClassCst (const ClassCst & cl) {
	    std::vector <Tree> results;
	    TreeStmtList pre = TreeStmtList::init ();
	    for (auto it : Ymir::r (0, cl.getTypes ().size ())) {
		auto val = castTo (cl.getTypes () [it], cl.getParameters () [it]);		
		results.push_back (val.getValue ());
		pre.append (val.getList ());
	    }

	    auto classType = generateType (cl.getType ());
	    if (cl.getSelf ().isEmpty ()) { // allocate the class
		TreeStmtList list (TreeStmtList::init ());
		Tree var = Tree::varDecl (cl.getLocation (), "self", classType);
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);

		auto vtable = generateVtable (cl.getType ().to <ClassPtr> ().getInners ()[0]);		
		auto classValue = Tree::affect (this-> stackVarDeclChain.back (), this-> getCurrentContext (), cl.getLocation (), var, Tree::buildCall (
		    cl.getLocation (),
		    classType,
		    global::CoreNames::get (CLASS_ALLOC),
		    {Tree::buildAddress (cl.getLocation (), vtable, Tree::pointerType (Tree::pointerType (Tree::voidType ())))}
		));
		
		results.insert (results.begin (), classValue);
	    } else {
		results.insert (results.begin (), generateValue (cl.getSelf ()));
	    }
	    
	    auto fn = generateValue (cl.getFrame ());
	    return Tree::compound ( // call the constructor
		cl.getLocation (),
		Tree::buildCall (cl.getLocation (), classType, fn, results),
		pre.toTree ()
	    );
	}

	generic::Tree Visitor::generateStructCst (const StructCst & cl) {
	    std::vector <std::string> names;
	    std::vector <Tree> results;
	    for (auto it : Ymir::r (0, cl.getTypes ().size ())) {
		names.push_back (cl.getStr ().to <generator::Struct> ().getFields () [it].getName ());
		results.push_back (castTo (cl.getTypes () [it], cl.getParameters () [it]));
	    }

	    auto type = generateType (cl.getType ());
	    return Tree::constructField (cl.getLocation (), type, names, results);
	}

	generic::Tree Visitor::generateUnionCst (const UnionCst & cl) {
	    std::vector <std::string> names;
	    std::vector <Tree> results;
	    names.push_back (cl.getFieldName ());
	    results.push_back (castTo (cl.getTypeCst (), cl.getParameter ()));	    
	    auto type = generateType (cl.getType ());
	    
	    return Tree::constructField (cl.getLocation (), type, names, results);
	}

	
	generic::Tree Visitor::generateCopier (const Copier & copy) {
	    auto inner = generateValue (copy.getWho ());
	    if (!copy.isAny ()) {
		if (copy.getType ().is <Array> ())
		    return inner;
		
		if (copy.getType ().is <Slice> ()) {
		    ulong size = generateType (copy.getType ().to <Slice> ().getInners () [0]).getSize ();
		    return Tree::buildCall (
			copy.getLocation (),
			generateType (copy.getType ()),
			global::CoreNames::get (DUPL_SLICE),
			{inner, Tree::buildSizeCst (size)}
		    );
		}
	    }

	    // Any type
	    auto innerType = generateType (copy.getWho ().to <Value> ().getType ());
	    auto ptrInnerType = Tree::pointerType (innerType);
	    inner =  Tree::buildAddress (copy.getLocation (), inner, ptrInnerType);
	    return Tree::buildCall (
		copy.getLocation (),
		generateType (copy.getType ()),
		global::CoreNames::get (DUPL_ANY),
		{inner, Tree::buildSizeCst (innerType.getSize ())}
	    );
	}

	generic::Tree Visitor::generateAliaser (const Aliaser & als) {
	    return castTo (als.getType (), als.getWho ());
	}	
	
	generic::Tree Visitor::generateArrayAccess (const ArrayAccess & access) {
	    auto left = generateValue (access.getArray ());
	    auto right = generateValue (access.getIndex ());

	    TreeStmtList list = TreeStmtList::init ();
	    list.append (left.getList ());
	    list.append (right.getList ());

	    auto lvalue = left.getValue (), rvalue = right.getValue ();
	    
	    return Tree::compound (
		access.getLocation (),
		Tree::buildArrayRef (access.getLocation (), lvalue, rvalue),
		list.toTree ()
	    );
	}
	
	generic::Tree Visitor::generateSliceAccess (const SliceAccess & access) {
	    auto left = generateValue (access.getSlice ());
	    auto right = generateValue (access.getIndex ());

	    TreeStmtList list = TreeStmtList::init ();
	    list.append (left.getList ());
	    list.append (right.getList ());

	    auto lvalue = left.getValue (), rvalue = right.getValue ();	    
	    ulong size = generateType (access.getSlice ().to <Value> ().getType ().to <Slice> ().getInners () [0]).getSize ();
	    
	    auto indexType = Tree::sizeType ();
	    auto index = Tree::binary (access.getLocation (), MULT_EXPR, indexType, rvalue, Tree::buildSizeCst (size));
	    auto type = Tree::pointerType (generateType (access.getType ()));
	    
	    auto data_field = lvalue.getField (Slice::PTR_NAME);
	   	    
	    auto value = Tree::binaryDirect (access.getLocation (), POINTER_PLUS_EXPR, type, data_field, index);
	    
	    return Tree::compound (
	    	access.getLocation (), 
		value.buildPointerUnref (0),
		list.toTree ()
	    );
	}

	generic::Tree Visitor::generateSliceConcat (const SliceConcat & slice) {
	    auto left = generateValue (slice.getLeft ());
	    auto right = generateValue (slice.getRight ());

	    auto type = generateType (slice.getType ());
	    ulong size = generateType (slice.getType ().to <Type> ().getInners () [0]).getSize ();
	    return Tree::buildCall (
		slice.getLocation (),
		type,
		global::CoreNames::get (SLICE_CONCAT),
		{left, right, Tree::buildSizeCst (size)}
	    );

	}

	generic::Tree Visitor::generateSizeOf (const SizeOf & size) {
	    if (size.getWho ().is <Void> ()) {
		return Tree::buildSizeCst (0);
	    } else {
		auto size_ = generateType (size.getWho ()).getSize ();
		return Tree::buildSizeCst (size_);
	    }
	}
	
	generic::Tree Visitor::castTo (const Generator & type, const Generator & val) {
	    auto value = generateValue (type, val);
	    if (type.is <Slice> ()) {
		auto inner = generateType (type.to <Slice> ().getInners () [0]);
		auto aux_type = Type::init (type.to<Type> (), type.to <Type> ().isMutable (), false);
		
		generic::Tree ret (generic::Tree::empty ());
		if (value.getType ().isStringType ()) {
		    auto chType = type.to <Slice> ().getInners ()[0];
		    ret = Tree::constructField (
			type.getLocation (),
			generateType (aux_type), 
			{Slice::LEN_NAME, Slice::PTR_NAME},
			{
			    value.getStringSize (chType.to<Char> ().getSize ()),
				value
				}
		    );
		} else if (val.to <Value> ().getType ().is <Array> ()) {
		    if (val.to<Value> ().getType ().to <Array> ().getInners ()[0].is <Void> ()) {
			ret = Tree::constructField (
			    type.getLocation (),
			    generateType (aux_type), 
			    {Slice::LEN_NAME, Slice::PTR_NAME},
			    {
				Tree::buildIntCst (val.getLocation (), (ulong) 0, Tree::intType (64, false)),
				    Tree::buildPtrCst (val.getLocation (), 0)
				    }
			);
		    } else {
			ret = Tree::constructField (
			    type.getLocation (),
			    generateType (aux_type), 
			    {Slice::LEN_NAME, Slice::PTR_NAME},
			    {
				value.getType ().getArraySize (),
				    Tree::buildAddress (type.getLocation (), value, Tree::pointerType (inner))
				    }
			);
		    }
		} else {
		    ret = value;
		    return value;
		}
		
		if (val.is <Copier> ()) {
		    auto list = ret.getList ();
		    ulong size = generateType (type.to <Slice> ().getInners () [0]).getSize ();
		    return
			Tree::compound (val.getLocation (), 
					Tree::buildCall (
					    val.getLocation (),
					    generateType (aux_type),
					    global::CoreNames::get (DUPL_SLICE),
					    {ret.getValue (), Tree::buildIntCst (val.getLocation (), size, Tree::intType (64, false))}						
					), list);
		} else return ret;
	    }
	    
	    return value;
	}
	
	void Visitor::enterBlock (const std::string & blockName) {
	    blockNames.push_back (blockName);
	    stackVarDeclChain.push_back (generic::TreeChain ());
	    stackBlockChain.push_back (generic::BlockChain ());
	}
	
	generic::TreeSymbolMapping Visitor::quitBlock (const lexing::Word & loc, const generic::Tree & content, const std::string & blockName) {
	    auto name = blockNames.back ();	    
	    auto varDecl = stackVarDeclChain.back ();
	    auto blockChain = stackBlockChain.back ();

	    blockNames.pop_back ();
	    stackBlockChain.pop_back ();
	    stackVarDeclChain.pop_back ();

	    if (name != blockName)
		Ymir::Error::halt ("", "");
	    
	    auto block = generic::Tree::block (loc, varDecl.first, blockChain.first);

	    if (!stackBlockChain.empty ()) {
		stackBlockChain.back ().append (block);
	    }

	    for (auto it : blockChain) {
		it.setBlockSuperContext (block);
	    }

	    auto bind = generic::Tree::build (BIND_EXPR, loc, generic::Tree::voidType (), varDecl.first, content, block);
	    return generic::TreeSymbolMapping (bind, block);
	}

	void Visitor::enterLoop (const Tree & label, const Tree & var) {
	    this-> _loopLabels.push_back (label);
	    this-> _loopVars.push_back (var);
	    
	    // If there is a break inside the loop it need to break, not go outside the loop to the closest catch
	    this-> exceptionDeclChain.back ().push_back ({Tree::empty (), Tree::empty (), Tree::empty (), Tree::empty ()});
	}

	void Visitor::quitLoop () {
	    this-> _loopVars.pop_back ();
	    this-> _loopLabels.pop_back ();
	    // cf enterloop
	    this-> exceptionDeclChain.back ().pop_back ();
	}	
	
	void Visitor::enterFrame () {
	    this-> _declarators.push_back ({});
	    this-> _throwLabels.push_back ({});
	    this-> exceptionDeclChain.push_back ({});
	}

	void Visitor::quitFrame () {
	    if (this-> _declarators.empty ()) {
		Ymir::Error::halt ("%(r) Quit non existing frame", "Critical");
	    }
	    
	    this-> _declarators.pop_back ();
	    this-> _throwLabels.pop_back ();
	    this-> exceptionDeclChain.pop_back ();
	}
	
	const generic::Tree & Visitor::getGlobalContext () {
	    if (this-> _globalContext.isEmpty ()) {
		this-> _globalContext = Tree::init (UNKNOWN_LOCATION, build_translation_unit_decl (NULL_TREE));
	    }
	    
	    return this-> _globalContext;
	}

	void Visitor::insertGlobalDeclarator (uint id, const generic::Tree & decl) {
	    this-> _globalDeclarators.emplace (id, decl);
	}
	
	void Visitor::insertDeclarator (uint id, const generic::Tree & decl) {
	    if (this-> _declarators.empty ()) {
		Ymir::Error::halt ("%(r) insert a declarator from outside a frame", "Critical");
	    }
	    
	    this-> _declarators.back ().emplace (id, decl);
	}

	Tree Visitor::getDeclarator (uint id) {
	    // if (this-> _declarators.empty ()) {
	    // 	Ymir::Error::halt ("%(r) get a declarator from outside a frame", "Critical");
	    // }
	    if (this-> _declarators.empty ()) {
		auto ptr = this-> _globalDeclarators.find (id);
		if (ptr == this-> _globalDeclarators.end ())
		    Ymir::Error::halt ("%(r) undefined declarators %(y)", "Critical", (int) id);
		return ptr-> second;
	    } else {
		auto ptr = this-> _declarators.back ().find (id);
		if (ptr == this-> _declarators.back ().end ()) {
		    ptr = this-> _globalDeclarators.find (id);
		    if (ptr == this-> _globalDeclarators.end ())
			Ymir::Error::halt ("%(r) undefined declarators %(y)", "Critical", (int) id);
		}
		return ptr-> second;
	    }	    
	}


	Tree Visitor::getDeclaratorOrEmpty (uint id) {
	    // if (this-> _declarators.empty ()) {
	    // 	Ymir::Error::halt ("%(r) get a declarator from outside a frame", "Critical");
	    // }

	    if (this-> _declarators.empty ()) {
		auto ptr = this-> _globalDeclarators.find (id);
		if (ptr == this-> _globalDeclarators.end ())
		    return Tree::empty ();
		return ptr-> second;
	    } else {
		auto ptr = this-> _declarators.back ().find (id);
		if (ptr == this-> _declarators.back ().end ()) {
		    ptr = this-> _globalDeclarators.find (id);
		    if (ptr == this-> _globalDeclarators.end ())
			return Tree::empty ();
		}
	    
	    
		return ptr-> second;
	    }
	}

	
	const generic::Tree & Visitor::getCurrentContext () const {	    
	    return this-> _currentContext;
	}

	void Visitor::setCurrentContext (const Tree & tr) {
	    this-> _currentContext = tr;
	}
	

	std::string Visitor::identify (const Generator & gen) {
	    match (gen) {
		of_u (Array, return "Array";);
		of_u (Bool, return "Bool";);
		of_u (Char, return "Char";);
		of_u (Float, return "Float";);
		of_u (Integer, return "Integer";);
		of_u (Slice, return "Slice";);
		of_u (Void, return "Void";);
		of_u (Affect, return "Affect";);
		of_u (Aliaser, return "Aliaser";);
		of_u (ArrayAccess, return "ArrayAccess";);
		of_u (ArrayValue, return "ArrayValue";);
		of_u (BinaryBool, return "BinaryBool";);
		of_u (BinaryFloat, return "BinaryFloat";);
		of_u (BinaryInt, return "BinaryInt";);
		of_u (Binary, return "Binary";);
		of_u (Block, return "Block";);
		of_u (BoolValue, return "BoolValue";);
		of_u (CharValue, return "CharValue";);
		of_u (Conditional, return "Conditional";);
		of_u (Copier, return "Copier";);
		of_u (Fixed, return "Fixed";);
		of_u (FloatValue, return "FloatValue";);
		of_u (None, return "None";);
		of_u (ParamVar, return "ParamVar";);
		of_u (Referencer, return "Referencer";);
		of_u (Set, return "Set";);
		of_u (SliceAccess, return "SliceAccess";);
		of_u (UnaryBool, return "UnaryBool";);
		of_u (UnaryFloat, return "UnaryFloat";);
		of_u (UnaryInt, return "UnaryInt";);
		of_u (Unary, return "Unary";);
		of_u (VarDecl, return "VarDecl";);
		of_u (VarRef, return "VarRef";);
	    }
	    return "empty";
	}

    }
    
}
