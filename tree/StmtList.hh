#pragma once

#include <ymir/tree/Tree.hh>

namespace generic {

    struct TreeStmtList {

	
	tree _list;
	
    private : 

	TreeStmtList ();
	
    public :

	static TreeStmtList init ();

	void append (const Tree & tree);

	Tree toTree () const;
	
    };        

}
