#pragma once

#include "Symbol.hh"
#include "Package.hh"
#include <vector>
#include <map>

namespace Semantic {

    struct Scope {

	SymbolPtr get (const std::string &);

	void insert (const std::string &, SymbolPtr);

	void addImport (Package * pck);
	
	
    private:

	std::map<std::string, SymbolPtr> local;
	std::vector <Package*> imported;
	
    };

}
