#include <ymir/semantic/generator/Visitor.hh>
#include <ymir/utils/Match.hh>
#include "toplev.h"

namespace semantic {

    namespace generator {

	using namespace generic;

	static GTY(()) vec<tree, va_gc> *globalDeclarations;
	
	Visitor::Visitor () :
	    _globalContext (Tree::empty ())
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
	}
	
	void Visitor::generateGlobalVar (const GlobalVar & var) {
	    auto type = generateType (var.getType ());
	    auto name = var.getName (); //Mangler::mangleGlobal (var.getName ());

	    Tree decl = Tree::varDecl (var.getLocation (), name, type);

	    decl.isStatic (true);
	    decl.isUsed (true);
	    decl.isExternal (false);
	    decl.preservePointer (true);
	    decl.isPublic (true);
	    decl.setDeclContext (getGlobalContext ());

	    vec_safe_push (globalDeclarations, decl.getTree ());
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
	
	void Visitor::enterBlock () {
	    stackVarDeclChain.push_back (generic::TreeChain ());
	    stackBlockChain.push_back (generic::BlockChain ());
	}
	
	generic::TreeSymbolMapping Visitor::quitBlock (const location_t & loc, const generic::Tree & content) {
	    auto varDecl = stackVarDeclChain.back ();
	    auto blockChain = stackBlockChain.back ();

	    stackBlockChain.pop_back ();
	    stackVarDeclChain.pop_back ();

	    auto block = generic::Tree::block (loc, varDecl.first, blockChain.first);

	    if (!stackBlockChain.empty ()) {
		stackBlockChain.back ().append (block);
	    }

	    for (tree it = blockChain.first.getTree () ; it != NULL_TREE ; it = BLOCK_CHAIN (it)) {
		BLOCK_SUPERCONTEXT (it) = block.getTree ();
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
	}

	
    }
    
}
