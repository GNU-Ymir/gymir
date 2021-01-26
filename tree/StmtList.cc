#include <ymir/tree/StmtList.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>
#include "config.h"
#include "system.h"
#include "coretypes.h"

#include "target.h"
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
#include <set>


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
