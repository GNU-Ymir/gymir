#include <ymir/tree/StmtList.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>
#include <set>
#include <ymir/ymir1.hh>

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
	    if (TREE_CODE (t.getTree ()) == COMPOUND_EXPR) {
	      this-> append (TREE_OPERAND (t.getTree (), 0));
	      this-> append (TREE_OPERAND (t.getTree (), 1));
	    } else {
	      this-> append (t.getTree ());
	    }
	}
    }

  void TreeStmtList::append (tree t) {
    if (TREE_CODE (t) == CLEANUP_POINT_EXPR) {
      TREE_TYPE (t) = void_type_node;
    }

    append_to_statement_list (t, &this-> _list);
  }

    bool TreeStmtList::isEmpty () const {
	return this-> _empty;
    }

    Tree TreeStmtList::toTree () const {
	return Tree::init (UNKNOWN_LOCATION, this-> _list);
    }    

}
