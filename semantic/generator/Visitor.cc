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
		list.append (value.getOperand (0));
		list.append (Tree::returnStmt (frame.getLocation (), resultDecl, value.getOperand (1)));
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
	}
	
	Tree Visitor::generateParamVar (const ParamVar & var) {
	    auto type = generateType (var.getType ());
	    auto name = var.getName ();
	    
	    auto decl = Tree::paramDecl (var.getLocation (), name, type);
	    
	    decl.setDeclContext (getCurrentContext ());
	    decl.setArgType (decl.getType ());
	    decl.isUsed (true);
	    
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

		of (Fixed, fixed,
		    return generateFixed (fixed);
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
		if (!last.isEmpty ())
		    list.append (last);
		return quitBlock (block.getLocation (), list.toTree ()).block;
	    }    
	}	
	
	Tree Visitor::generateFixed (const Fixed & fixed) {
	    auto type = generateType (fixed.getType ());
	    if (fixed.getType ().to <Integer> ().isSigned ()) 
		return Tree::buildIntCst (fixed.getLocation (), fixed.getUI ().i, type);
	    else
		return Tree::buildIntCst (fixed.getLocation (), fixed.getUI ().u, type);
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
	    // ?
	}

	void Visitor::quitFrame () {
	    // ?
	}

	const generic::Tree & Visitor::getGlobalContext () {
	    if (this-> _globalContext.isEmpty ()) {
		this-> _globalContext = Tree::init (UNKNOWN_LOCATION, build_translation_unit_decl (NULL_TREE));
	    }
	    
	    return this-> _globalContext;
	}
	
	const generic::Tree & Visitor::getCurrentContext () const {	    
	    return this-> _currentContext;
	}

	void Visitor::setCurrentContext (const Tree & tr) {
	    this-> _currentContext = tr;
	}
	
    }
    
}
