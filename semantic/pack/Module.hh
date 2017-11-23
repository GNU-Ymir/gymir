#pragma once

#include <gc/gc_cpp.h>
#include "Namespace.hh"
#include "Scope.hh"
#include "Symbol.hh"
#include <ymir/utils/Array.hh>

namespace semantic {

    class IModule : public gc {

	Namespace _space;
	std::vector <Namespace> _opens;
	std::vector <Namespace> publicOpens;
	Scope globalScope;
	
    public:

	IModule (Namespace);

	Symbol get (std::string name);

	std::vector <Symbol> getAll (std::string name);

	void insert (Symbol sym);

	void addOpen (Namespace space);

	void close (Namespace space);

	void addPublicOpen (Namespace space);

	std::vector <Namespace> opens ();

	std::vector <Namespace> accessible ();

	bool authorized (Namespace);

	Namespace space ();

    private:

	std::vector <Namespace> accessible (std::vector <Namespace>);
	
    };

    typedef IModule* Module;
    
}
