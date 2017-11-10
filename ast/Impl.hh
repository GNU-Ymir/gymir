#pragma once

#include "Declaration.hh"
#include "Constructor.hh"
#include <vector>
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IFunction;
    typedef IFunction* Function;

    class IConstructor;
    typedef IConstructor* Constructor;
    
    class IImpl : public IDeclaration {

	std::vector <Function> methods;
	std::vector <Constructor> csts;
	std::vector <bool> herit;
	Word who, what;

    public:

	IImpl (Word what, std::vector <Function> methods, std::vector <Constructor> csts) :
	    methods (methods),
	    csts (csts),
	    who (Word::eof ()),
	    what (what)
	{}

	
	IImpl (Word what, Word who, std::vector <Function> methods, std::vector <bool> herit, std::vector <Constructor> csts) :
	    methods (methods),
	    csts (csts),
	    herit (herit),
	    who (who),
	    what (what)
	{}

	void print (int nb = 0) override;
	
    };

    typedef IImpl* Impl;

}
