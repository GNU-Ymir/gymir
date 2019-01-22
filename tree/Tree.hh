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

#include <ymir/lexing/Word.hh>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>

#include <vector>
#include <map>


namespace generic {

    /**
     * This class is a form of proxy for generic tree
     * Unlike all other proxy this one is only referencing 
     * Meaning a tree can be in multiple instance of Tree
     */
    struct Tree {

	tree _t;
	
	location_t _loc;

    private :
	
	Tree ();

    public :

	static Tree init (const location_t & loc, const tree & t);
	    		
	static Tree error ();

	static Tree empty ();

	static Tree block (const location_t & loc, const Tree & vars, const Tree & chain);

	static Tree voidType ();

	static Tree varDecl (const location_t & loc, const std::string & name, const Tree & type);

	static Tree varDecl (const lexing::Word & loc, const std::string & name, const Tree & type);
	
	/**
	 * \brief Build a tree of 1 operands
	 */
	static Tree build (tree_code tc, const location_t & loc, const Tree & type, const Tree & t1);
    
	/**
	 * \brief Build a tree of 2 operands
	 */
	static Tree build (tree_code tc, const location_t & loc, const Tree & type, const Tree & t1, const Tree & t2);

	/**
	 * \brief Build a tree of 3 operands
	 */
	static Tree build (tree_code tc, const location_t & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3);

	/**
	 * \brief Build a tree of 4 operands
	 */
	static Tree build (tree_code tc, const location_t & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4);

	/**
	 * \brief Build a tree of 5 operands
	 */
	static Tree build (tree_code tc, location_t loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4, const Tree & t5);

	/**
	 * \return the location of the tree (for debuging infos)
	 */
	const location_t & getLocus () const;

	/**
	 * \brief set the location of the tree (for debuging infos)
	 */
	void setLocus (const location_t & loc);

	/**
	 * \brief Break the proxy, and return the generic gimple tree
	 */
	const tree & getTree () const;

	/**
	 * \brief Break the proxy, and set the generic gimple tree
	 */
	void setTree (const tree & t);

	/**
	 * \return the kind of tree 
	 */
	tree_code getTreeCode () const;
	
	bool isError () const;

	bool isEmpty () const;

	void isStatic (bool);

	void isUsed (bool);

	void isExternal (bool);

	void preservePointer (bool);

	void isPublic (bool);

	void setDeclContext (const Tree & context);
	
	/**
	 * \return the type of this tree
	 */
	Tree getType () const;

	/**
	 * \return the i eme operand of this tree
	 */
	Tree getOperand (int i) const;
	
    };
   
    inline bool operator == (Tree t1, Tree t2) {
	return t1.getTree () == t2.getTree ();
    }

    inline bool operator != (Tree t1, Tree t2) {
	return !(t1 == t2);
    }


    // struct TreeStmtList {

    // public:

    // 	TreeStmtList () : list (alloc_stmt_list ()) {}
    // 	TreeStmtList (Tree t) : list (t.getTree ()) {}

    // 	void append (Tree t) {
    // 	    if (!t.isNull ())
    // 		append_to_statement_list (t.getTree (), &list);
    // 	}

    // 	tree getTree () const {
    // 	    return list;
    // 	}

    // private:

    // 	tree list;
	
    // };

    // template <typename Append>
    // struct TreeChainBase {

    // 	Tree first;
    // 	Tree last;

    // 	TreeChainBase ()
    // 	    : first (),
    // 	      last ()
    // 	{}


    // 	void append (Tree t) {
    // 	    gcc_assert (!t.isNull());
    // 	    if (first.isNull ()) {
    // 		first = last = t;
    // 	    } else {
    // 		Append () (last, t);
    // 		last = t;
    // 	    }
    // 	}	
    // };

    // struct tree_chain_append {
    // 	void operator () (Tree t, Tree a) {
    // 	    TREE_CHAIN (t.getTree ()) = a.getTree ();
    // 	}
    // };


    // struct block_chain_append {
    // 	void operator () (Tree t, Tree a) {
    // 	    BLOCK_CHAIN (t.getTree ()) = a.getTree ();
    // 	}
    // };
    
    // struct TreeChain : TreeChainBase <tree_chain_append> {};
    // struct BlockChain : TreeChainBase <block_chain_append> {};


    // struct TreeSymbolMapping {
    // 	Tree bind_expr;
    // 	Tree block;
	
    // 	TreeSymbolMapping (Tree bind, Tree block) :
    // 	    bind_expr (bind),
    // 	    block (block) {
    // 	}
    // };

}
