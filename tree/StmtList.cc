#include <ymir/tree/StmtList.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>
#include <set>
#include <ymir/ymir1.hh>

namespace generic {
    
    TreeStmtList::TreeStmtList () :
	_list (alloc_stmt_list ()),
	_empty (true)
    {}

    TreeStmtList TreeStmtList::init () {
	return TreeStmtList ();
    }

    void TreeStmtList::append (const Tree & t) {
	if (!t.isEmpty ()) {
	    this-> _empty = false;
	    append_to_statement_list (t.getTree (), &(this-> _list));
	}
    }

    bool TreeStmtList::isEmpty () const {
	return this-> _empty;
    }

    Tree TreeStmtList::toTree () const {
	return Tree::init (UNKNOWN_LOCATION, this-> _list);
    }    

}
