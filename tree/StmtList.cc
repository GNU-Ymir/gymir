#include <ymir/tree/StmtList.hh>

namespace generic {

    TreeStmtList::TreeStmtList () :
	_list (alloc_stmt_list ())
    {}

    TreeStmtList TreeStmtList::init () {
	return TreeStmtList ();
    }

    void TreeStmtList::append (const Tree & t) {
	if (!t.isEmpty ()) {
	    append_to_statement_list (t.getTree (), &(this-> _list));
	}
    }

    Tree TreeStmtList::toTree () const {
	return Tree::init (UNKNOWN_LOCATION, this-> _list);
    }    

}
