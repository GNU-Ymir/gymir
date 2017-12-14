#pragma once

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
#include "tree-pretty-print.h"
#include "print-tree.h"

#include <vector>

namespace Ymir {

    struct Tree {
	Tree ()	    
	    : t (NULL_TREE),
	      loc (UNKNOWN_LOCATION)
	{}

	Tree (tree t_)
	    : t (t_),
	      loc (EXPR_LOCATION (t))
	{}

	Tree (tree t_, location_t loc)
	    : t(t_),
	      loc (loc)
	{}

	Tree (Tree t_, location_t loc)
	    : t(t_.getTree ()),
	      loc (loc)
	{}

	location_t getLocus () const {
	    return loc;
	}

	void setLocus (location_t loc) {
	    this->loc = loc;
	}

	tree getTree () const {
	    return t;
	}

	tree_code getTreeCode () const {
	    return TREE_CODE (t);
	}	 
	
	void setTree (tree t) {
	    this->t = t;
	}

	bool isError () const {
	    return error_operand_p (t);
	}

	bool isNull () {
	    return t == NULL_TREE;
	}

	static Tree error () {
	    return Tree (error_mark_node);
	}

	Tree getType () const {
	    return TREE_TYPE (t);
	}

    private:

	tree t;
	location_t loc;
	
	
    };
   
    inline bool operator == (Tree t1, Tree t2) {
	return t1.getTree () == t2.getTree ();
    }

    inline bool operator != (Tree t1, Tree t2) {
	return !(t1 == t2);
    }

    inline Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1) {
	return build1_loc (loc, tc, type.getTree (), t1.getTree ());
    }

    inline Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2) {
	return build2_loc (loc, tc, type.getTree (), t1.getTree(), t2.getTree ());
    }

    inline Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2, Tree t3) {
	return build3_loc (loc, tc, type.getTree (), t1.getTree(), t2.getTree (), t3.getTree ());
    }

    inline Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2, Tree t3, Tree t4) {
	return build4_loc (loc, tc, type.getTree (), t1.getTree(), t2.getTree (), t3.getTree (), t4.getTree ());
    }

    inline Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2, Tree t3, Tree t4, Tree t5) {
	return build5_loc (loc, tc, type.getTree (), t1.getTree(), t2.getTree (), t3.getTree (), t4.getTree (), t5.getTree ());
    }


    struct TreeStmtList {

    public:

	TreeStmtList () : list (alloc_stmt_list ()) {}
	TreeStmtList (Tree t) : list (t.getTree ()) {}

	void append (Tree t) {
	    append_to_statement_list (t.getTree (), &list);
	}

	tree getTree () const {
	    return list;
	}

    private:

	tree list;
	
    };

    template <typename Append>
    struct TreeChainBase {

	Tree first;
	Tree last;

	TreeChainBase ()
	    : first (),
	      last ()
	{}


	void append (Tree t) {
	    gcc_assert (!t.isNull());
	    if (first.isNull ()) {
		first = last = t;
	    } else {
		Append () (last, t);
		last = t;
	    }
	}	
    };

    struct tree_chain_append {
	void operator () (Tree t, Tree a) {
	    TREE_CHAIN (t.getTree ()) = a.getTree ();
	}
    };


    struct block_chain_append {
	void operator () (Tree t, Tree a) {
	    BLOCK_CHAIN (t.getTree ()) = a.getTree ();
	}
    };
    
    struct TreeChain : TreeChainBase <tree_chain_append> {};
    struct BlockChain : TreeChainBase <block_chain_append> {};


    struct TreeSymbolMapping {
	Tree bind_expr;
	Tree block;
	
	TreeSymbolMapping (Tree bind, Tree block) :
	    bind_expr (bind),
	    block (block) {
	}
    };

    std::vector <TreeChain>& getStackVarDeclChain ();
    std::vector <TreeStmtList>& getStackStmtList ();
    std::vector <BlockChain>& getStackBlockChain ();
    
    void enterBlock ();
    TreeSymbolMapping leaveBlock ();    
    Tree& currentContext ();
};
