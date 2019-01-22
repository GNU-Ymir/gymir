#pragma once

#include <ymir/tree/Tree.hh>

namespace generic {

    template <typename Append>
    struct TreeChainBase {

	Tree first;
	Tree last;

	TreeChainBase ()
	    : first (Tree::empty ()),
	      last (Tree::empty ())
	{}

	void append (Tree t) {
	    if (t.isEmpty ()) 
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	    
	    if (first.isEmpty ()) {
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

}
