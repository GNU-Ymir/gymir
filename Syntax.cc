#include "Syntax.hh"
#include "AstGC.hh"

namespace Syntax {

    AstGC AstGC::inst;
    
    unsigned long AstGC::addInfo (Ast * elem) {
	this->table.push_back (elem);
	return this-> table.size () - 1;
    }

    void AstGC::free (unsigned long id) {
	delete this-> table [id];
	this-> table [id] = NULL;
    }
    
    void AstGC::clean () {
	for (auto & it : this->table) {
	    if (it != NULL)
		delete it;
	}
	this->table.clear();
    }

    namespace AstEnums {
	const char * toString (AstEnum elem) {
	    switch (elem) {
#define AST_ENUM (name, descr)			\
	    case name:				\
		return descr;
#define AST_ENUM_KEYWORD (x, y) AST_ENUM (x, y)
#undef AST_ENUM_KEYWORD
#undef AST_ENUM
	    default: return NULL;
	    }
	}
    }
};
