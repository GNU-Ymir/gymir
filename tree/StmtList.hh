#pragma once

#include <ymir/tree/Tree.hh>

namespace generic {

    struct TreeStmtList {

	
	tree _list;

	bool _empty;
	
    private : 

	TreeStmtList ();
	
    public :

	static TreeStmtList init ();

	void append (const Tree & tree);

	Tree toTree () const;

	bool isEmpty () const;
	
    };        

}
