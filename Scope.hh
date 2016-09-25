#pragma once

#include "Symbol.hh"
#include "Package.hh"
#include <vector>
#include <map>

namespace Semantic {

    struct Scope {

	Symbol & get (const std::string &);

	void insert (const std::string &, Symbol);

	void addImport (Package * pck);
	
	
    private:

	std::map<std::string, Symbol> local;
	std::vector <Package*> imported;
	
    };

}
