#pragma once

#include "FrameScope.hh"
#include <list>
#include <vector>
#include "TypeInfo.hh"

namespace Semantic {

    struct Table {
	
	static Table & instance () {
	    return inst;	    
	}

	void enterBlock ();

	void quitBlock ();

	void enterFrame (const std::string&);

	void quitFrame ();

	TypeInfo *& return_type ();
	
	void addImport (Package * pck);

	void insert (const std::string &name, SymbolPtr sym);

	SymbolPtr get (const std::string &name);	

	std::string space () const;

	bool alreadyReturned () const;
	
    private:
	
	Table () {}
	Table(const Table&);
	Table & operator = (const Table &);
	
	static Table inst;
	Scope globalScope;
	std::list <FrameScope> frameTable;
	std::list <TypeInfo*> return_types;
	
    };
       
}
