#include "Syntax.hh"

namespace Syntax {

    Ast::Ast (TokenPtr token) : token (token) {
	AstGC::instance().addInfo (this);
    }
    
    void AstGC::addInfo (Ast * elem) {
	this->table.push_back (elem);
    }

    void AstGC::empty() {
	for (auto & it ; this->table) {
	    delete it;
	}
	this->table.clear();
    }
    
    
};
