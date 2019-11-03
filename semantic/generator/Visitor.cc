#include <ymir/semantic/generator/Visitor.hh>
#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/semantic/symbol/Struct.hh>
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
	
	Visitor::Visitor () :
	    _globalContext (Tree::empty ()),
	    _currentContext (Tree::empty ())
	{}

	Visitor Visitor::init () {
	    return Visitor ();
	}

	void Visitor::finalize () {
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
			 
		else of (StructRef, st, {
			auto gen = st.getRef ().to <semantic::Struct> ().getGenerator ();
			std::vector <Tree> inner;
			std::vector <std::string> fields;
			for (auto & it : gen.to <generator::Struct> ().getFields ()) {
			    inner.push_back (generateType (it.to <generator::VarDecl> ().getVarType ()));
			    fields.push_back (it.to <generator::VarDecl> ().getName ());
			}
			type = Tree::tupleType (fields, inner);
		    }
		)
			 
		else of (EnumRef, en, {
			auto _type = en.getRef ().to <semantic::Enum> ().getGenerator ().to <generator::Enum> ().getType ();
			type = generateType (_type);			
		    });
	    }
	    
	    if (type.isEmpty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");

	    if (gen.to <Type> ().isRef ())
		type = Tree::pointerType (type);
	    
	    return type;
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
			{"len", "ptr"},
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
	    auto name = var.getName (); //Mangler::mangleGlobal (var.getName ());

	    Tree decl = Tree::varDecl (var.getLocation (), name, type);

	    decl.isStatic (true);
	    decl.isUsed (true);
	    decl.isExternal (false);
	    decl.isPreserved (true);
	    decl.isPublic (true);
	    decl.setDeclContext (getGlobalContext ());

	    vec_safe_push (globalDeclarations, decl.getTree ());
	}
	
	void Visitor::generateMainCall (bool isVoid, const std::string & mainName) {
	    auto argcT = Tree::intType (64, false);
	    auto argvT = Tree::pointerType (Tree::pointerType (Tree::charType (8)));
	    std::vector <Tree> args = {argcT, argvT};
	    
	    auto ret = Tree::intType (32, true);

	    Tree fnType = Tree::functionType (ret, args);
	    Tree fn_decl = Tree::functionDecl (lexing::Word::eof (), Keys::MAIN, fnType);
	    auto asmName = Keys::MAIN;
	    fn_decl.asmName (asmName);

	    setCurrentContext (fn_decl);
	    enterFrame ();

	    auto argc = generateParamVar ("argc", argcT);
	    auto argv = generateParamVar ("argv", argvT);
	    std::list <Tree> argsList = {argc, argv};

	    fn_decl.setDeclArguments (argsList);

	    enterBlock ();
	    auto resultDecl = Tree::resultDecl (lexing::Word::eof (), ret);
	    fn_decl.setResultDecl (resultDecl);
	    TreeStmtList list = TreeStmtList::init ();
	    std::string name;
	    if (global::State::instance ().isDebugActive ()) {
		name = global::CoreNames::get (RUN_MAIN_DEBUG);
	    } else name = global::CoreNames::get (RUN_MAIN);
	    

	    Tree mainRet = ret;
	    if (isVoid) mainRet = Tree::voidType ();

	    auto proto = Tree::buildFrameProto (lexing::Word::eof (), mainRet, mainName, {});
	    
	    auto call = Tree::buildCall (
		lexing::Word::eof (),
		mainRet,
		name,
		{argc, argv, proto}
	    );

	    if (!isVoid)
		list.append (Tree::returnStmt (lexing::Word::eof (), resultDecl, call));
	    else {
		list.append (call);
		list.append (Tree::returnStmt (
		    lexing::Word::eof (), resultDecl,
		    Tree::buildIntCst (lexing::Word::eof (), (ulong) 0, ret)
		));
	    }
	    
	    Tree value = list.toTree ();

	    auto fnTree = quitBlock (lexing::Word::eof (), value);
	    auto fnBlock = fnTree.block;
	    fnBlock.setBlockSuperContext (fn_decl);

	    fn_decl.setDeclInitial (fnBlock);
	    fn_decl.setDeclSavedTree (fnTree.bind_expr);
	    fn_decl.isExternal (false);
	    fn_decl.isPreserved (true);
	    fn_decl.isPublic (true);
	    fn_decl.isStatic (true);

	    gimplify_function_tree (fn_decl.getTree ());
	    cgraph_node::finalize_function (fn_decl.getTree (), true);
	    setCurrentContext (Tree::empty ());
	    quitFrame ();	    	    
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
	   	    
	    if (frame.getName () == Keys::MAIN) 
		generateMainCall (frame.getType ().is <Void> (), asmName);
	    
	    setCurrentContext (fn_decl);
	    enterFrame ();

	    std::list <Tree> arglist;
	    for (auto & p : frame.getParams ())
		arglist.push_back (generateParamVar (p.to<ParamVar> ()));
	    
	    fn_decl.setDeclArguments (arglist);
	    
	    enterBlock ();
	    auto resultDecl = Tree::resultDecl (frame.getLocation (), ret);
	    fn_decl.setResultDecl (resultDecl);

	    Tree value (Tree::empty ());
	    if (frame.needFinalReturn ()) {
		TreeStmtList list = TreeStmtList::init ();
		value = castTo (frame.getType (), frame.getContent ());
		list.append (value.getList ());
		value = value.getValue ();
		if (!resultDecl.getType ().isPointerType ()) value = value.toDirect ();

		list.append (Tree::returnStmt (frame.getLocation (), resultDecl, value));
		value = list.toTree ();
	    } else value = generateValue (frame.getContent ());
	    
	    auto fnTree = quitBlock (lexing::Word::eof (), value);
	    auto fnBlock = fnTree.block;
	    fnBlock.setBlockSuperContext (fn_decl);	    
	    
	    fn_decl.setDeclInitial (fnBlock);	    
	    fn_decl.setDeclSavedTree (fnTree.bind_expr);

	    fn_decl.isExternal (false);
	    fn_decl.isPreserved (true);
	    fn_decl.isWeak (frame.isWeak ());

	    fn_decl.isPublic (true);
	    fn_decl.isStatic (true);

	    gimplify_function_tree (fn_decl.getTree ());
	    cgraph_node::finalize_function (fn_decl.getTree (), true);
	    setCurrentContext (Tree::empty ());
	    quitFrame ();
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
		
		else of (VarRef, var,
		    return generateVarRef (var);
		)

		else of (VarDecl, decl,
		    return generateVarDecl (decl);
		)

		else of (Referencer, _ref,
		    return generateReferencer (_ref);
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

		else of (UnaryBool, ub,
		    return generateUnaryBool (ub);
		)

		else of (UnaryInt, ui,
		    return generateUnaryInt (ui);
		)

		else of (UnaryFloat, uf,
		    return generateUnaryFloat (uf);
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

		else of (FrameProto, pr,
		    return generateFrameProto (pr);
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

		else of (StructRef, rf ATTRIBUTE_UNUSED,
		    return Tree::empty ();
		);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point %(y)", "Critical", identify (gen));
	    return Tree::empty ();
	}
	
	Tree Visitor::generateBlock (const Block & block) {
	    TreeStmtList list = TreeStmtList::init ();
	    Generator last (Generator::empty ());
	    Tree var (Tree::empty ());
	    if (!block.getType ().is<Void> ()) {
		var = Tree::varDecl (block.getLocation (), "_", generateType (block.getType ()));
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);
	    }

	    enterBlock ();
	    for (auto & it : block.getContent ()) {
		if (!last.isEmpty ()) list.append (generateValue (last));
		last = it;
	    }

	    if (!block.getType ().is<Void> ()) {
		auto value = castTo (block.getType (), last);
		list.append (value.getList ());
		value = value.getValue ();
		if (!var.getType ().isPointerType ()) value = value.toDirect ();

		list.append (Tree::affect (block.getLocation (), var, value));		
		auto binding = quitBlock (block.getLocation (), list.toTree ());
		return Tree::compound (block.getLocation (),
				       var, 
				       binding.bind_expr);
	    } else {
		if (!last.isEmpty ())
		    list.append (generateValue (last));
		return quitBlock (block.getLocation (), list.toTree ()).bind_expr;
	    }    
	}	

	Tree Visitor::generateSet (const Set & set) {
	    TreeStmtList list = TreeStmtList::init ();
	    Tree last (Tree::empty ());
	    Tree var (Tree::empty ());
	    if (!set.getType ().is<Void> ()) {
		var = Tree::varDecl (set.getLocation (), "_", generateType (set.getType ()));
	    }

	    for (auto & it : set.getContent ()) {
		if (!last.isEmpty ()) list.append (last);
		last = generateValue (it);
	    }

	    if (!set.getType ().is<Void> ()) {
		list.append (Tree::affect (set.getLocation (), var, last));
		auto binding = list.toTree ();
		return Tree::compound (set.getLocation (),
				       var, 
				       binding);
	    } else {
		list.append (last);
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
	    return Tree::buildFloatCst (f.getLocation (), f.getValue (), type);
	}

	Tree Visitor::generateChar (const CharValue & c) {
	    auto type = generateType (c.getType ());
	    return Tree::buildCharCst (c.getLocation (), c.getValue (), type);
	}
	
	Tree Visitor::generateAffect (const Affect & aff) {
	    auto left = generateValue (aff.getWho ());
	    auto right = castTo (aff.getWho ().to <Value> ().getType (), aff.getValue ());

	    TreeStmtList list = TreeStmtList::init ();
	    list.append (left.getList ());
	    list.append (right.getList ());
	    auto lvalue = aff.getWho ().to <Value> ().getType ().to <Type> ().isRef () ? left.getValue ().toDirect () : left.getValue ();
	    auto rvalue = aff.getValue ().to <Value> ().getType ().to <Type> ().isRef () ? right.getValue ().toDirect () : right.getValue ();
	    
	    auto value = Tree::affect (aff.getLocation (), lvalue, rvalue);
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

	    auto lvalue = left.getValue ().toDirect (); // We want an int, but it can be a ref to a int
	    auto rvalue = right.getValue ().toDirect ();
	    
	    auto value = Tree::binary (bin.getLocation (), code, type, lvalue, rvalue);	    
	    auto ret = Tree::compound (bin.getLocation (), value, list.toTree ());
	    return ret;
	}

	Tree Visitor::generateBinaryBool (const BinaryBool & bin) {
	    auto left = generateValue (bin.getLeft ());
	    auto right = generateValue (bin.getRight ());
	    TreeStmtList list = TreeStmtList::init ();

	    list.append (left.getList ());
	    list.append (right.getList ());

	    tree_code code = LSHIFT_EXPR; // Fake default affectation to avoid warning
	    switch (bin.getOperator ()) {
	    case Binary::Operator::AND : code = TRUTH_ANDIF_EXPR; break;
	    case Binary::Operator::OR : code = TRUTH_ORIF_EXPR; break;
	    case Binary::Operator::EQUAL : code = EQ_EXPR; break;
	    case Binary::Operator::NOT_EQUAL : code = NE_EXPR; break;
	    default :
		Ymir::Error::halt ("%(r) - unhandeld case", "Critical");
	    }

	    auto lvalue = left.getValue ().toDirect ();
	    auto rvalue = right.getValue ().toDirect ();
	    
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
	    switch (bin.getOperator ()) {;
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
	    
	    auto lvalue = left.getValue ().toDirect ();
	    auto rvalue = right.getValue ().toDirect ();
	    
	    auto type = generateType (bin.getType ());
	    auto value = Tree::binary (bin.getLocation (), code, type, lvalue, rvalue);
	    auto ret = Tree::compound (bin.getLocation (), value, list.toTree ());
	    return ret;	    
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

	    value = value.getValue ().toDirect ();

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
		value = value.getValue ().toDirect ();

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
	
	generic::Tree Visitor::generateVarRef (const VarRef & var) {
	    return this-> getDeclarator (var.getRefId ());
	}	

	generic::Tree Visitor::generateVarDecl (const VarDecl & var) {
	    auto type = generateType (var.getVarType ());
	    auto name = var.getName ();

	    auto decl = Tree::varDecl (var.getLocation (), name, type);
	    if (!var.getVarValue ().isEmpty ()) {
		if (!var.getVarType ().to <Type> ().isRef ())
		    decl.setDeclInitial (castTo (var.getVarType (), var.getVarValue ()).toDirect ());
		else
		    decl.setDeclInitial (generateValue (var.getVarValue ()));		
	    } else if (var.isAutoInit ())
		decl.setDeclInitial (generateInitValueForType (var.getVarType ()));	    

	    decl.setDeclContext (getCurrentContext ());
	    stackVarDeclChain.back ().append (decl);
	    
	    insertDeclarator (var.getUniqId (), decl);
	    return Tree::declExpr (var.getLocation (), decl);
	}

	generic::Tree Visitor::generateReferencer (const Referencer & ref) {
	    auto inner = generateValue (ref.getWho ());
	    if (ref.getWho ().to <Value> ().getType ().to <Type> ().isRef ())
		return inner;

	    auto type = generateType (ref.getType ());
	    return Tree::buildAddress (ref.getLocation (), inner, type);
	}	

	generic::Tree Visitor::generateConditional (const Conditional & cond) {
	    auto test = generateValue (cond.getTest ());
	    Tree var (Tree::empty ());
	    if (!cond.getType ().is<Void> ()) {
		var = Tree::varDecl (cond.getLocation (), "_", generateType (cond.getType ()));
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);
	    }
	    	    
	    Tree content (Tree::empty ());
	    if (!var.isEmpty ()) {
		TreeStmtList list = TreeStmtList::init ();
		content = castTo (cond.getType (), cond.getContent ());
		list.append (content.getList ());
		auto value = content.getValue ();
		if (!var.getType ().isPointerType ()) value = value.toDirect ();
		
		list.append (Tree::affect (cond.getLocation (), var, value));
		content = list.toTree ();
	    } else content = generateValue (cond.getContent ());
	    
	    auto elsePart = Tree::empty ();
	    if (!cond.getElse ().isEmpty ()) {
		if (!var.isEmpty ()) {
		    TreeStmtList list = TreeStmtList::init ();
		    elsePart = castTo (cond.getType (), cond.getElse ());
		    list.append (elsePart.getList ());
		    auto value = elsePart.getValue ();
		    if (!var.getType ().isPointerType ()) value = value.toDirect ();
		
		    list.append (Tree::affect (cond.getLocation (), var, value));
		    elsePart = list.toTree ();
		} else elsePart = generateValue (cond.getElse ());
	    }

	    return Tree::compound (cond.getLocation (),
				   var,
				   Tree::conditional (cond.getLocation (), getCurrentContext (), test, content, elsePart)
	    );
	}

	generic::Tree Visitor::generateLoop (const Loop & loop) {
	    Tree var (Tree::empty ());
	    Tree test (Tree::empty ());
	    if (!loop.getTest ().isEmpty ())
		test = generateValue (loop.getTest ());
	    if (!loop.getType ().is<Void> ()) {
		var = Tree::varDecl (loop.getLocation (), "_", generateType (loop.getType ()));
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);
	    }

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
		if (!var.getType ().isPointerType ()) value = value.toDirect ();
		list.append (Tree::affect (loop.getLocation (), var, value));
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
	    return Tree::compound (loop.getLocation (),
				  var, 
				  all.toTree ()
	    );
	}

	generic::Tree Visitor::generateBreak (const Break & br) {
	    TreeStmtList list = TreeStmtList::init ();
	    if (!br.getValue ().to <Value> ().getType ().is<Void> ()) {
	    	auto value = generateValue (br.getValue ());		
	    	list.append (
	    	    Tree::affect (br.getLocation (),
	    			  this-> _loopVars.back (),
	    			  value)
	    	);
	    }
	    	    
	    list.append (Tree::gotoExpr (br.getLocation (), this-> _loopLabels.back ()));
	    return list.toTree ();
	}
	
	generic::Tree Visitor::generateArrayValue (const ArrayValue & val) {
	    auto type = generateType (val.getType ());
	    auto inner = generateType (val.getType ().to <Array> ().getInners () [0]);
	    std::vector <Tree> params;
	    for (auto it : val.getContent ()) {
		auto value = inner.isPointerType () ? generateValue (it) : generateValue (it).toDirect ();
		params.push_back (value);
	    }
	    return Tree::constructIndexed (val.getLocation (), type, params);
	}

	generic::Tree Visitor::generateTupleValue (const TupleValue & val) {
	    auto type = generateType (val.getType ());
	    std::vector <Tree> params;
	    for (auto it : val.getContent ()) {
		auto inner = generateType (it.to<Value> ().getType ());
		auto value = inner.isPointerType () ? generateValue (it) : generateValue (it).toDirect ();
		params.push_back (value);
	    }
	    return Tree::constructField (val.getLocation (), type, {}, params);
	}	

	generic::Tree Visitor::generateStringValue (const StringValue & str) {
	    auto inner = str.getType ().to <Array> ().getInners () [0];
	    auto len = str.getLen ();
	    auto size = inner.to<Char> ().getSize ();
	    const char * data = str.getValue ().data ();
	    println ((long) data, " ", len, " ", size);
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

	generic::Tree Visitor::generateTupleAccess (const TupleAccess & acc) {
	    auto elem = castTo (acc.getTuple ().to <Value> ().getType (), acc.getTuple ());
	    auto field = Ymir::format ("_%", acc.getIndex ());
	    return elem.toDirect ().getField (field);
	}

	generic::Tree Visitor::generateStructAccess (const StructAccess & acc) {
	    auto elem = castTo (acc.getStruct ().to <Value> ().getType (), acc.getStruct ());
	    return elem.toDirect ().getField (acc.getField ());
	}

	generic::Tree Visitor::generateCall (const Call & cl) {
	    std::vector <Tree> results;
	    for (auto it : Ymir::r (0, cl.getTypes ().size ())) {
		results.push_back (castTo (cl.getTypes () [it], cl.getParameters () [it]));
		auto type = generateType (cl.getTypes () [it]);
		if (!type.isPointerType ())
		    results.back () = results.back ().toDirect ();
	    }

	    auto fn = generateValue (cl.getFrame ());
	    auto type = generateType (cl.getType ());
	    return Tree::buildCall (cl.getLocation (), type, fn , results);
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
	
	generic::Tree Visitor::generateCopier (const Copier & copy) {
	    auto inner = generateValue (copy.getWho ()).toDirect ();
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
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Tree::empty ();
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

	    auto lvalue = left.getValue ().toDirect (), rvalue = right.getValue ().toDirect ();
	    
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

	    auto lvalue = left.getValue ().toDirect (), rvalue = right.getValue ().toDirect ();
	    ulong size = generateType (access.getSlice ().to <Value> ().getType ().to <Slice> ().getInners () [0]).getSize ();
	    
	    auto indexType = Tree::sizeType ();
	    auto index = Tree::binary (access.getLocation (), MULT_EXPR, indexType, rvalue, Tree::buildSizeCst (size));
	    auto data_field = lvalue.getField ("ptr");
	    
	    auto ptr = Tree::binaryDirect (access.getLocation (), POINTER_PLUS_EXPR, data_field.getType (), data_field, index);
	    return ptr.buildPointerUnref (0);
	}
	
	generic::Tree Visitor::castTo (const Generator & type, const Generator & val) {
	    auto value = generateValue (val);
	    if (type.is <Slice> ()) {
		auto inner = generateType (type.to <Slice> ().getInners () [0]);
		auto aux_type = type;
		aux_type.to <Type> ().isRef (false);
		
		generic::Tree ret (generic::Tree::empty ());
		if (value.getType ().isStringType ()) {
		    auto chType = type.to <Slice> ().getInners ()[0];
		    ret = Tree::constructField (
			type.getLocation (),
			generateType (aux_type), 
			{"len", "ptr"},
			{
			    value.getStringSize (chType.to<Char> ().getSize ()),
			    value
			}
		    );
		} else if (val.to <Value> ().getType ().is <Array> ()) {		    
		    ret = Tree::constructField (
			type.getLocation (),
			generateType (aux_type), 
			{"len", "ptr"},
			{
			    value.getType ().getArraySize (),
			    Tree::buildAddress (type.getLocation (), value, Tree::pointerType (inner))
			}
		    );		    
		} else {
		    ret = value;
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
	
	void Visitor::enterBlock () {
	    stackVarDeclChain.push_back (generic::TreeChain ());
	    stackBlockChain.push_back (generic::BlockChain ());
	}
	
	generic::TreeSymbolMapping Visitor::quitBlock (const lexing::Word & loc, const generic::Tree & content) {
	    auto varDecl = stackVarDeclChain.back ();
	    auto blockChain = stackBlockChain.back ();

	    stackBlockChain.pop_back ();
	    stackVarDeclChain.pop_back ();

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
	}

	void Visitor::quitLoop () {
	    this-> _loopVars.pop_back ();
	    this-> _loopLabels.pop_back ();
	}	
	
	void Visitor::enterFrame () {
	    this-> _declarators.push_back ({});
	}

	void Visitor::quitFrame () {
	    if (this-> _declarators.empty ()) {
		Ymir::Error::halt ("%(r) Quit non existing frame", "Critical");
	    }
	    
	    this-> _declarators.pop_back ();
	}
	
	const generic::Tree & Visitor::getGlobalContext () {
	    if (this-> _globalContext.isEmpty ()) {
		this-> _globalContext = Tree::init (UNKNOWN_LOCATION, build_translation_unit_decl (NULL_TREE));
	    }
	    
	    return this-> _globalContext;
	}

	void Visitor::insertDeclarator (uint id, const generic::Tree & decl) {
	    if (this-> _declarators.empty ()) {
		Ymir::Error::halt ("%(r) insert a declarator from outside a frame", "Critical");
	    }
	    
	    this-> _declarators.back ().emplace (id, decl);
	}

	Tree Visitor::getDeclarator (uint id) {
	    if (this-> _declarators.empty ()) {
		Ymir::Error::halt ("%(r) get a declarator from outside a frame", "Critical");
	    }

	    auto ptr = this-> _declarators.back ().find (id);
	    if (ptr == this-> _declarators.back ().end ()) {
		Ymir::Error::halt ("%(r) undefined declarators %(y)", "Critical", (int) id);
	    }

	    return ptr-> second;
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
