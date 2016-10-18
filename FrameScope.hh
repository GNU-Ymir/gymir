#pragma once

#include <list>
#include <vector>
#include "Scope.hh"

namespace Semantic {

    struct FrameScope {
	
	FrameScope (const std::string& space);
	
	void enterBlock ();
	
	void quitBlock ();
	
	void addImport (Package * pck);
	
	void insert (const std::string & name, SymbolPtr symbol);
	
	SymbolPtr get (const std::string & name);

	std::string space () const;
	
    private:
	
	std::list <Scope> local;
	std::string space_name;
	std::vector<Package*> imported;
    };
}
