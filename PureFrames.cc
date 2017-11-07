#include "PureFrames.hh"
#include "UndefInfo.hh"
#include "VoidInfo.hh"
#include "Table.hh"

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


namespace Semantic {

    std::vector<PureFrame> PureFrames::frames;
    std::vector<FinalFrame> PureFrames::final_frames;

    void PureFrame::verifyReturn () {
    }

    void PureFrame::setMainParams (std::vector<Syntax::VarPtr> &) {	
    }
    
    void PureFrame::validate () {
	Syntax::Function * final = new Syntax::Function (fun -> token, space);
	Table::instance ().enterFrame (space);
	
	std::vector <Syntax::VarPtr> params;
	if (fun-> token-> getStr () == "main")
	    setMainParams (params);
	else  {
	    for (int i = 0; i < (int)fun->params.size (); i++) {
		params.push_back (fun->params[i]->declare (fun->token->getCstr ()));
	    }
	}
	    
	if (fun->type == Syntax::Var::empty ()) 
	    Table::instance().return_type () = new UndefInfo ();
	else {
	    auto type = fun-> type-> asType ();
	    if (type == NULL) return;
	    else 
		Table::instance().return_type () = type-> getType ()-> type;
	}
	    
	Syntax::BlockPtr block = (Syntax::BlockPtr)fun-> block-> instruction ();
	    
	auto info = Table::instance ().return_type ();
	Table::instance ().quitFrame ();
	if (info == NULL || info-> Is (UNDEF)) {
	    info = new VoidInfo;
	    //TODO verifier l'existance du type de retour
	} else verifyReturn ();
	    
	final-> info = info;
	final-> params = params;
	final-> block = block;
	
	PureFrames::insertFinal (FinalFrame (final, space));
    }

    void FinalFrame::finalize () {

	tree main_fndecl_type_params [] = {
	};
	
	tree ident = get_identifier ("main");
	tree fntype = build_function_type_array (integer_type_node, 0, main_fndecl_type_params);
	tree fn_decl = build_decl (BUILTINS_LOCATION, FUNCTION_DECL, ident, fntype);


	DECL_EXTERNAL (fn_decl) = 0;
	TREE_PUBLIC (fn_decl) = 1;
	TREE_STATIC (fn_decl) = 1;
	
	Ymir::enterBlock ();

	this-> fun-> block-> toGeneric ();
	
	tree result_decl = build_decl (BUILTINS_LOCATION, RESULT_DECL,
				       NULL_TREE, integer_type_node);

	DECL_RESULT (fn_decl) = result_decl;

	tree set_result = build2 (INIT_EXPR, void_type_node, DECL_RESULT (fn_decl),
				  build_int_cst_type (integer_type_node, 0));
	tree return_stmt = build1 (RETURN_EXPR, void_type_node, set_result);

	Ymir::getStackStmtList ().back ().append (return_stmt);      

	auto fnTreeBlock = Ymir::leaveBlock ();
	auto fnBlock = fnTreeBlock.block;

	BLOCK_SUPERCONTEXT (fnBlock.getTree ()) = fn_decl;
	DECL_INITIAL (fn_decl) = fnBlock.getTree ();
	DECL_SAVED_TREE (fn_decl) = fnTreeBlock.bind_expr.getTree ();
	
	gimplify_function_tree (fn_decl);
	cgraph_node::finalize_function (fn_decl, true);
	fn_decl = NULL_TREE;
    }

           
}
