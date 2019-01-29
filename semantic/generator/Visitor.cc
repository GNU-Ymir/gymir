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
	    match (gen) {
		of (Integer, i,
		    return generateIntegerType (i);
		);		

		of (Void, v ATTRIBUTE_UNUSED,
		    return Tree::voidType ();
		);

		of (Bool, b ATTRIBUTE_UNUSED,
		    return Tree::boolType ();
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
		list.append (Tree::returnStmt (frame.getLocation (), resultDecl, value.getValue ()));
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

	Tree Visitor::generateIntegerType (const Integer & type) {
	    if (type.isSigned ()) {
		switch (type.getSize ()) {
		case 8 : return Tree::init (type.getLocation ().getLocus (), signed_char_type_node);
		case 16 : return Tree::init (type.getLocation ().getLocus (), short_integer_type_node);
		case 32 : return Tree::init (type.getLocation ().getLocus (), integer_type_node);
		case 64 : return Tree::init (type.getLocation ().getLocus (), long_integer_type_node);
		}
	    } else {
		switch (type.getSize ()) {
		case 8 : return Tree::init (type.getLocation ().getLocus (), unsigned_char_type_node);
		case 16 : return Tree::init (type.getLocation ().getLocus (), short_unsigned_type_node);
		case 32 : return Tree::init (type.getLocation ().getLocus (), unsigned_type_node);
		case 64 : return Tree::init (type.getLocation ().getLocus (), long_unsigned_type_node);
		}
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Tree::empty ();
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

		of (BinaryInt, i,
		    return generateBinaryInt (i);
		);

		of (BinaryBool, b,
		    return generateBinaryBool (b);
		);
		
		of (VarRef, var,
		    return generateVarRef (var);
		);

		of (VarDecl, decl,
		    return generateVarDecl (decl);
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
	    }

	    enterBlock ();
	    for (auto & it : block.getContent ()) {
		if (!last.isEmpty ()) list.append (last);
		last = generateValue (it);
	    }

	    if (!block.getType ().is<Void> ()) {
		list.append (Tree::affect (block.getLocation (), var, last));
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
	    
	    auto value = Tree::binary (bin.getLocation (), code, type, left.getValue (), right.getValue ());	    
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

	    auto type = generateType (bin.getType ());
	    auto value = Tree::binary (bin.getLocation (), code, type, left.getValue (), right.getValue ());
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
	    if (!var.getVarValue ().isEmpty ())
		decl.setDeclInitial (generateValue (var.getVarValue ()));

	    decl.setDeclContext (getCurrentContext ());
	    stackVarDeclChain.back ().append (decl);
	    
	    insertDeclarator (var.getUniqId (), decl);
	    return Tree::declExpr (var.getLocation (), decl);
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
