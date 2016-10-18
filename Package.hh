#pragma once

#include <map>
#include "Symbol.hh"

namespace Semantic {

    struct Package {

	std::string filename;	

	std::map <std::string, SymbolPtr> global_declarations;	
	
	Package (std::string filename) : filename (filename) {}

	void addSymbol (std::string name, Symbol symbol);
	
	Symbol getSymbol (const std::string &name) const;	

    };    

};
