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
    
    
};
