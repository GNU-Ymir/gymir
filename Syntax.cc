#include "Syntax.hh"
#include "AstGC.hh"

namespace Syntax {

    AstGC AstGC::inst;
    
    void AstGC::addInfo (Ast * elem) {
	this->table.push_back (elem);
    }

    
    void AstGC::clean () {
	for (auto & it : this->table) {
	    delete it;
	}
	this->table.clear();
    }
    
    
};
