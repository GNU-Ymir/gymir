#include "Scope.hh"

namespace Semantic {
    
    Symbol & Scope::get (const std::string & name) {
	auto it = local.find (name);
	if (it == local.end ()) {
	    return Symbol::empty ();
	} else {
	    return it->second;
	}
    }

    void Scope::insert (const std::string & name, Symbol sym) {
	this->local.insert( std::pair < std::string, Symbol > (name, sym));
    }

    void Scope::addImport (Package * pck) {
	this->imported.push_back (pck);
    }    

}
