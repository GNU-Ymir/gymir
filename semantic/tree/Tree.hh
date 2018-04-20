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

#include "convert.h"
#include "langhooks.h"
#include "ubsan.h"
#include "stringpool.h"
#include "attribs.h"
#include "asan.h"
#include "tree-pretty-print.h"
#include "print-tree.h"
#include "cppdefault.h"
#include "tm.h"

#include <vector>
#include <map>

#define ubyte_type_node unsigned_char_type_node
#define byte_type_node signed_char_type_node
#define ushort_type_node short_unsigned_type_node
#define short_type_node short_integer_type_node
#define int_type_node integer_type_node
#define uint_type_node unsigned_type_node
#define long_type_node long_integer_type_node
#define ulong_type_node long_unsigned_type_node

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

	Tree getOperand (int i) const {
	    return TREE_OPERAND (t, i);
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

    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1);

    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2);

    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2, Tree t3);

    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2, Tree t3, Tree t4);

    Tree buildTree (tree_code tc, location_t loc, Tree type, Tree t1, Tree t2, Tree t3, Tree t4, Tree t5);

    struct TreeStmtList {

    public:

	TreeStmtList () : list (alloc_stmt_list ()) {}
	TreeStmtList (Tree t) : list (t.getTree ()) {}

	void append (Tree t) {
	    if (!t.isNull ())
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
    std::map <std::string, Tree>& getLoopLabelsNamed ();
    std::vector <Tree>& getLoopLabels ();
    
    
    void enterBlock ();
    TreeSymbolMapping leaveBlock ();    
    Tree& currentContext ();
};
