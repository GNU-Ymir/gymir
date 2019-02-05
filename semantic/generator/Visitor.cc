#include <ymir/semantic/generator/Visitor.hh>
#include <ymir/utils/Match.hh>
#include "toplev.h"

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
		);		

		of (Void, v ATTRIBUTE_UNUSED,
		    type = Tree::voidType ();
		);

		of (Bool, b ATTRIBUTE_UNUSED,
		    type = Tree::boolType ();
		);
		
		of (Float, f,
		    type = Tree::floatType (f.getSize ());
		);

		of (Char, c,
		    type = Tree::charType (c.getSize ());
		);

		of (Array, array,
		    type = generateArrayType (array);
		);
	    }
	    
	    if (type.isEmpty ())
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");

	    if (gen.to <Type> ().isRef ())
		type = Tree::pointerType (type);
	    
	    return type;
	}

	Tree Visitor::generateArrayType (const Array & array) {
	    if (array.size () != -1) {
		return Tree::staticArray (generateType (array.getInner ()), array.size ());
	    } else {
		return Tree::dynArray (generateType (array.getInner ()));
	    }
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

	void Visitor::generateFrame (const Frame & frame) {
	    std::vector <Tree> args;
	    for (auto i : Ymir::r (0, args.size ())) {
		args.push_back (generateType (frame.getParams () [i].to<ParamVar> ().getType ()));
	    }

	    Tree ret = generateType (frame.getType ());	    
	    Tree fntype = Tree::functionType (ret, args);
	    Tree fn_decl = Tree::functionDecl (frame.getLocation (), frame.getName (), fntype);

	    setCurrentContext (fn_decl);
	    enterFrame ();

	    std::list <Tree> arglist;
	    for (auto & p : frame.getParams ())
		arglist.push_back (generateParamVar (p.to<ParamVar> ()));
	    
	    fn_decl.setDeclArguments (arglist);
	    
	    enterBlock ();
	    auto resultDecl = Tree::resultDecl (frame.getLocation (), ret);
	    fn_decl.setResultDecl (resultDecl);

	    auto value = generateValue (frame.getContent ());
	    if (!frame.getType ().is<Void> () && frame.getType ().equals (frame.getContent ().to <Value> ().getType ())) {
		TreeStmtList list = TreeStmtList::init ();
		list.append (value.getList ());
		value = value.getValue ();
		if (!resultDecl.getType ().isPointerType ()) value = value.toDirect ();
		
		list.append (Tree::returnStmt (frame.getLocation (), resultDecl, value));
		value = list.toTree ();
	    }
	    
	    auto fnTree = quitBlock (lexing::Word::eof (), value);
	    auto fnBlock = fnTree.block;
	    fnBlock.setBlockSuperContext (fn_decl);	    
	    
	    fn_decl.setDeclInitial (fnBlock);	    
	    fn_decl.setDeclSavedTree (fnTree.bind_expr);

	    fn_decl.isExternal (false);
	    fn_decl.isPreserved (true);
	    fn_decl.isWeak (true);

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
	
	Tree Visitor::generateValue (const Generator & gen) {
	    match (gen) {
		of (Block, block,
		    return generateBlock (block);
		);

		of (Set, set,
		    return generateSet (set);
		);
		
		of (Fixed, fixed,
		    return generateFixed (fixed);
		);

		of (BoolValue, b,
		    return generateBool (b);
		);

		of (FloatValue, f,
		    return generateFloat (f);
		);

		of (CharValue, c,
		    return generateChar (c);
		);
		
		of (Affect, aff,
		    return generateAffect (aff);
		);
		
		of (BinaryInt, i,
		    return generateBinaryInt (i);
		);

		of (BinaryBool, b,
		    return generateBinaryBool (b);
		);

		of (BinaryFloat, f,
		    return generateBinaryFloat (f);
		);
		
		of (VarRef, var,
		    return generateVarRef (var);
		);

		of (VarDecl, decl,
		    return generateVarDecl (decl);
		);

		of (Referencer, _ref,
		    return generateReferencer (_ref);
		);

		of (Conditional, cond,
		    return generateConditional (cond);
		);

		of (ArrayValue, val,
		    return generateArrayValue (val);
		);

		of (Copier, copy,
		    return generateCopier (copy);
		);

		of (None, none ATTRIBUTE_UNUSED,
		    return Tree::empty ();
		);
	    }

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Tree::empty ();
	}
	
	Tree Visitor::generateBlock (const Block & block) {
	    TreeStmtList list = TreeStmtList::init ();
	    Tree last (Tree::empty ());
	    Tree var (Tree::empty ());
	    if (!block.getType ().is<Void> ()) {
		var = Tree::varDecl (block.getLocation (), "_", generateType (block.getType ()));
		var.setDeclContext (getCurrentContext ());
		stackVarDeclChain.back ().append (var);
	    }

	    enterBlock ();
	    for (auto & it : block.getContent ()) {
		if (!last.isEmpty ()) list.append (last);
		last = generateValue (it);
	    }

	    if (!block.getType ().is<Void> ()) {
		list.append (last.getList ());
		auto value = last.getValue ();
		if (!var.getType ().isPointerType ()) value = value.toDirect ();

		list.append (Tree::affect (block.getLocation (), var, value));		
		auto binding = quitBlock (block.getLocation (), list.toTree ());
		return Tree::compound (block.getLocation (),
				       var, 
				       binding.bind_expr);
	    } else {
		list.append (last);
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
	    if (fixed.getType ().to <Integer> ().isSigned ()) 
		return Tree::buildIntCst (fixed.getLocation (), fixed.getUI ().i, type);
	    else
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
	    auto right = generateValue (aff.getValue ());

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
	
	generic::Tree Visitor::generateVarRef (const VarRef & var) {
	    return this-> getDeclarator (var.getRefId ());
	}	

	generic::Tree Visitor::generateVarDecl (const VarDecl & var) {
	    auto type = generateType (var.getVarType ());
	    auto name = var.getName ();

	    auto decl = Tree::varDecl (var.getLocation (), name, type);
	    if (!var.getVarValue ().isEmpty ()) {
		if (!var.getVarType ().to <Type> ().isRef ())
		    decl.setDeclInitial (generateValue (var.getVarValue ()).toDirect ());
		else
		    decl.setDeclInitial (generateValue (var.getVarValue ()));		
	    } else 
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
	    }

	    auto content = generateValue (cond.getContent ());
	    if (!var.isEmpty ()) {
		TreeStmtList list = TreeStmtList::init ();
		list.append (content.getList ());
		auto value = content.getValue ();
		if (!var.getType ().isPointerType ()) value = value.toDirect ();
		
		list.append (Tree::affect (cond.getLocation (), var, value));
		content = list.toTree ();
	    }
	    
	    auto elsePart = Tree::empty ();
	    if (!cond.getElse ().isEmpty ()) {
		elsePart = generateValue (cond.getElse ());
		if (!var.isEmpty ()) {
		    TreeStmtList list = TreeStmtList::init ();
		    list.append (elsePart.getList ());
		    auto value = elsePart.getValue ();
		    if (!var.getType ().isPointerType ()) value = value.toDirect ();
		
		    list.append (Tree::affect (cond.getLocation (), var, value));
		    elsePart = list.toTree ();
		}	
	    }

	    return Tree::compound (cond.getLocation (),
				   var,
				   Tree::conditional (cond.getLocation (), getCurrentContext (), test, content, elsePart)
	    );
	}

	generic::Tree Visitor::generateArrayValue (const ArrayValue & val) {
	    auto type = generateType (val.getType ());
	    std::vector <Tree> params;
	    for (auto it : val.getContent ()) {
		params.push_back (generateValue (it));
	    }
	    return Tree::constructIndexed (val.getLocation (), type, params);
	}

	generic::Tree Visitor::generateCopier (const Copier & copy) {
	    auto inner = generateValue (copy.getWho ());
	    if (copy.getType ().is <Array> () && copy.getType ().to <Array> ().isStatic ())
		return inner;
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Tree::empty ();
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
	
    }
    
}
