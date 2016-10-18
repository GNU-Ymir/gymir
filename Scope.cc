#include "Scope.hh"

namespace Semantic {
    
    SymbolPtr Scope::get (const std::string & name) {
	auto it = local.find (name);
	if (it == local.end ()) {
	    return Symbol::empty ();
	} else {
	    return it->second;
	}
    }

    void Scope::insert (const std::string & name, SymbolPtr sym) {
	this->local.insert( std::pair < std::string, SymbolPtr > (name, sym));
    }

    void Scope::addImport (Package * pck) {
	this->imported.push_back (pck);
    }    

}
