#include <ymir/semantic/tree/Tree.hh>
#include "print-tree.h"
#include <iostream>


namespace Ymir {

    std::vector <TreeChain> stack_var_decl_chain;
    std::vector <TreeStmtList> stack_stmt_list;
    std::vector <BlockChain> stack_block_chain;
    
    void enterBlock () {
	stack_stmt_list.push_back (TreeStmtList ());
	stack_var_decl_chain.push_back (TreeChain ());
	stack_block_chain.push_back (BlockChain ());	
    }

    TreeSymbolMapping leaveBlock () {
	printf ("leave\n");
	TreeStmtList current = stack_stmt_list.back ();
	stack_stmt_list.pop_back ();

	TreeChain var_decl_chain = stack_var_decl_chain.back ();
	stack_var_decl_chain.pop_back ();

	BlockChain subblocks = stack_block_chain.back ();
	stack_block_chain.pop_back ();

	tree new_block = build_block (var_decl_chain.first.getTree (),
				      subblocks.first.getTree (),
				      NULL_TREE, NULL_TREE
	);

	if (!stack_block_chain.empty ()) {
	    stack_block_chain.back().append (new_block);
	}

	for (tree it = subblocks.first.getTree (); it != NULL_TREE ; it = BLOCK_CHAIN (it)) {
	    BLOCK_SUPERCONTEXT (it) = new_block;
	}

	tree bind_expr = build3 (
	    BIND_EXPR, void_type_node, var_decl_chain.first.getTree (),
	    current.getTree (), new_block
	);
	
	return TreeSymbolMapping (bind_expr, new_block);
    }
    
    std::vector <TreeChain>& getStackVarDeclChain () {
	return stack_var_decl_chain;
    }
    
    std::vector <TreeStmtList>& getStackStmtList () {
	return stack_stmt_list;
    }
    
    std::vector <BlockChain>& getStackBlockChain () {
	return stack_block_chain;
    }
    
    Tree& currentContext () {
	static Tree ctx;
	return ctx;
    }

}
