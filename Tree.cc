#include <ymir/semantic/tree/Tree.hh>
#include "print-tree.h"
#include <iostream>


namespace Ymir {

    std::vector <TreeChain> stack_var_decl_chain;
    std::vector <TreeStmtList> stack_stmt_list;
    std::vector <BlockChain> stack_block_chain;
    std::map <std::string, Tree> loopLabelsNamed;
    std::vector <Tree> loopLabels;
    std::vector <std::map <void*, Ymir::Tree> > assocTree;
    
    void enterBlock () {
	stack_stmt_list.push_back (TreeStmtList ());
	stack_var_decl_chain.push_back (TreeChain ());
	stack_block_chain.push_back (BlockChain ());	
    }

    TreeSymbolMapping leaveBlock () {
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

    std::map <std::string, Tree> & getLoopLabelsNamed () {
	return loopLabelsNamed;
    }

    std::vector <Tree> & getLoopLabels () {
	return loopLabels;
    }

    std::map <void*, Tree> & getAssocTree () {
	return assocTree.back ();
    }

    void enterFrame () {
	assocTree.push_back ({});
    }

    void quitFrame () {
	assocTree.pop_back ();
    }
    
    Tree& currentContext () {
	static Tree ctx;
	return ctx;
    }
    
    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1) {
	return build1_loc (loc, tc, type.getTree (), t1.getTree ());
    }

    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2) {
	return build2_loc (loc, tc, type.getTree (), t1.getTree(), t2.getTree ());
    }

    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2, Tree t3) {
	return build3_loc (loc, tc, type.getTree (), t1.getTree(), t2.getTree (), t3.getTree ());
    }

    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2, Tree t3, Tree t4) {
	return build4_loc (loc, tc, type.getTree (), t1.getTree(), t2.getTree (), t3.getTree (), t4.getTree ());
    }

    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2, Tree t3, Tree t4, Tree t5) {
	return build5_loc (loc, tc, type.getTree (), t1.getTree(), t2.getTree (), t3.getTree (), t4.getTree (), t5.getTree ());
    }

    
}
