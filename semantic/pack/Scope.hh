#pragma once

#include "Symbol.hh"
#include "Namespace.hh"

#include <map>
#include <string>
#include <vector>

namespace semantic {

    class Scope {

	std::map <std::string, std::vector <Symbol> > local;
	std::vector <Namespace> imports;

    public:

	Scope ();

	Symbol operator [] (std::string name) const;
	
	Symbol& operator [] (std::string name);

	std::vector <Symbol> getAll (std::string name);

	void addOpen (Namespace space);

	Symbol getAlike (std::string name);

	void clear ();

	void quit (Namespace space);
	
	std::string toString ();
	
    };
       
}
