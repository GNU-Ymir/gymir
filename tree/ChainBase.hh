#pragma once

#include <ymir/tree/Tree.hh>

namespace generic {

    template <typename NEXT>
    struct ChainIterator {

	Tree current;

	ChainIterator (Tree begin) :
	    current (begin)
	{}
	
	Tree operator* () {
	    return current;
	}

	void operator++ () {
	    current = (NEXT ()).next (current);	    
	}

	bool operator!= (const ChainIterator<NEXT> & ot) {
	    return current != ot.current;
	}	
	
    };
    
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

	ChainIterator<Append> begin () {
	    return ChainIterator<Append> {first};
	}

	ChainIterator<Append> end () const {
	    return ChainIterator<Append> {Tree::empty ()};
	}
	
    };

    struct tree_chain_append {
	void operator () (Tree t, Tree a) {
	    TREE_CHAIN (t.getTree ()) = a.getTree ();
	}

	Tree next (Tree t) {
	    return Tree::init (BUILTINS_LOCATION, TREE_CHAIN (t.getTree ()));
	}

	tree next (tree t) {
	    return TREE_CHAIN (t);
	}
    };


    struct block_chain_append {
	void operator () (Tree t, Tree a) {
	    BLOCK_CHAIN (t.getTree ()) = a.getTree ();
	}

	Tree next (Tree t) {
	    return Tree::init (BUILTINS_LOCATION, BLOCK_CHAIN (t.getTree ()));
	}

	tree next (tree t) {
	    return BLOCK_CHAIN (t);
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
