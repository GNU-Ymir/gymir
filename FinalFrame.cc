#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/ast/Var.hh>
#include <ymir/ast/Block.hh>
#include <ymir/ast/Expression.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/utils/Mangler.hh>

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"
#include "tree.h"
#include "tree-iterator.h"
#include "input.h"
#include "diagnostic.h"
#include "stringpool.h"
#include "cgraph.h"
#include "gimplify.h"
#include "gimple-expr.h"
#include "convert.h"
#include "print-tree.h"
#include "stor-layout.h"
#include "fold-const.h"
#include "print-tree.h"

namespace semantic {

    Ymir::Tree IFinalFrame::__fn_decl__;
    std::map <std::string, Ymir::Tree> IFinalFrame::__declared__;
    std::vector <Ymir::Tree> IFinalFrame::__contextToAdd__;

    IFinalFrame::IFinalFrame (Symbol type, Namespace space, std::string name, const std::vector<syntax::Var> & vars, syntax::Block bl, const std::vector <syntax::Expression>& tmps) :
	_type (type),
	_file (""),
	_space (space),
	_name (name),
	_vars (vars),
	_tmps (tmps),
	_isVariadic (false),
	_block (bl)
    {}
    
    std::string IFinalFrame::name () {
	return this-> _name;
    }

    Namespace IFinalFrame::space () {
	return this-> _space;
    }

    std::string& IFinalFrame::file () {
	return this-> _file;
    }

    Symbol IFinalFrame::type () {
	return this-> _type;
    }

    bool& IFinalFrame::isVariadic () {
	return this-> _isVariadic;
    }

    std::vector<syntax::Var>& IFinalFrame::vars () {
	return this-> _vars;
    }

    std::vector <syntax::Expression>& IFinalFrame::tmps () {
	return this-> _tmps;
    }

    syntax::Block IFinalFrame::block () {
    	return this-> _block;
    }
    
    Ymir::Tree IFinalFrame::currentFrame () {
	return __fn_decl__;
    }

    Ymir::Tree IFinalFrame::getDeclaredType (const char* name) {
	auto type = __declared__.find (name);
	if (type == __declared__.end ()) return Ymir::Tree ();
	return type-> second;
    }
    
    void IFinalFrame::declareType (std::string &name, Ymir::Tree type) {
	if (__declared__.find (name) == __declared__.end ()) {
	    Ymir::Tree decl = build_decl (BUILTINS_LOCATION, TYPE_DECL,
					  get_identifier (name.c_str ()),
					  type.getTree ()
	    );

	    if (__fn_decl__.isNull ()) {
		__contextToAdd__.push_back (decl);
	    } else {
		DECL_CONTEXT (decl.getTree ()) = __fn_decl__.getTree ();
		Ymir::getStackVarDeclChain ().back ().append (decl);
	    }
	    
	    __declared__ [name] = type;
	}	    
    }

    void IFinalFrame::declareType (const char* name, Ymir::Tree type) {
	if (__declared__.find (name) == __declared__.end ()) {
	    Ymir::Tree decl = build_decl (BUILTINS_LOCATION, TYPE_DECL,
					  get_identifier (name),
					  type.getTree ()
	    );


	    if (__fn_decl__.isNull ()) {
		__contextToAdd__.push_back (decl);
	    } else {
		DECL_CONTEXT (decl.getTree ()) = __fn_decl__.getTree ();
		Ymir::getStackVarDeclChain ().back ().append (decl);
	    }
	   	    
	    __declared__ [name] = type;
	}	    
    }

    void IFinalFrame::declArguments () {
	Ymir::Tree arglist;
	for (auto var : this-> _vars) {
	    Ymir::Tree decl = build_decl (
		var-> token.getLocus (),
		PARM_DECL,
		get_identifier (var-> token.getStr ().c_str ()),
		var-> info-> type-> toGeneric ().getTree ()
	    );

	    DECL_CONTEXT (decl.getTree ()) = __fn_decl__.getTree ();	    
	    DECL_ARG_TYPE (decl.getTree ()) = TREE_TYPE (decl.getTree ());
	    
	    arglist = chainon (arglist.getTree (), decl.getTree ());
	    TREE_USED (decl.getTree ()) = 1;
	    var-> info-> treeDecl (decl);
	}
	DECL_ARGUMENTS (__fn_decl__.getTree ()) = arglist.getTree ();	
    }

    void IFinalFrame::finalize () {
	ISymbol::resetNbTmp ();
	__declared__.clear ();
	__contextToAdd__.clear ();
	
	std::vector <tree> args (this-> _vars.size ());
	for (uint i = 0 ; i < this-> _vars.size () ; i++)
	    args [i] = this-> _vars [i]-> info-> type-> toGeneric ().getTree ();

	tree ret = this-> _type-> type-> toGeneric ().getTree ();
		
	//tree ident = get_identifier (this-> _name.c_str ());
	tree ident = get_identifier (Mangler::mangle_function (this-> _name, this).c_str ());
	
	tree fntype = build_function_type_array (ret, args.size (), args.data ());
	tree fn_decl = build_decl (BUILTINS_LOCATION, FUNCTION_DECL, ident, fntype);	
	//DECL_ASSEMBLER_NAME (fn_decl) = asmIdent;
	
	Ymir::currentContext () = fn_decl;
	__fn_decl__ = fn_decl;

	this-> declArguments ();
	
	Ymir::enterBlock ();
	tree result_decl = build_decl (BUILTINS_LOCATION, RESULT_DECL,
				       NULL_TREE, ret);
	
	DECL_RESULT (fn_decl) = result_decl;
	
	Ymir::Tree inside = this-> _block-> toGeneric ();	
	Ymir::getStackStmtList ().back ().append (inside);	
	
	auto fnTreeBlock = Ymir::leaveBlock ();
	auto fnBlock = fnTreeBlock.block;

	BLOCK_SUPERCONTEXT (fnBlock.getTree ()) = fn_decl;
	DECL_INITIAL (fn_decl) = fnBlock.getTree ();
	DECL_SAVED_TREE (fn_decl) = fnTreeBlock.bind_expr.getTree ();

	DECL_EXTERNAL (fn_decl) = 0;
	DECL_PRESERVE_P (fn_decl) = 1;

	TREE_PUBLIC (fn_decl) = 1;
	TREE_STATIC (fn_decl) = 1;
	
	gimplify_function_tree (fn_decl);
	cgraph_node::finalize_function (fn_decl, true);
	fn_decl = NULL_TREE;
	__fn_decl__ = NULL_TREE;
    }
    
    
}
