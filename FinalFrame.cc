#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/ast/Var.hh>
#include <ymir/ast/Block.hh>
#include <ymir/ast/Expression.hh>
#include <ymir/semantic/tree/Tree.hh>

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

namespace semantic {
    IFinalFrame::IFinalFrame (Symbol type, Namespace space, std::string name, std::vector <syntax::Var> vars, syntax::Block bl, std::vector <syntax::Expression> tmps) :
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

    std::vector<syntax::Var> IFinalFrame::vars () {
	return this-> _vars;
    }

    std::vector <syntax::Expression> IFinalFrame::tmps () {
	return this-> _tmps;
    }

    syntax::Block IFinalFrame::block () {
    	return this-> _block;
    }

    void IFinalFrame::finalize () {
	tree fndecl_type_params [] = {
	};

	tree ident = get_identifier (this-> _name.c_str ());
	tree fntype = build_function_type_array (void_type_node, 0, fndecl_type_params);
	tree fn_decl = build_decl (BUILTINS_LOCATION, FUNCTION_DECL, ident, fntype);

	Ymir::enterBlock ();
	this-> _block-> toGenericNoEntry ();
	tree result_decl = build_decl (BUILTINS_LOCATION, RESULT_DECL,
				       NULL_TREE, void_type_node);
	DECL_RESULT (fn_decl) = result_decl;

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
    }
    
    
}
