#pragma once

#include <ymir/utils/memory.hh>
#include "Namespace.hh"
#include "Scope.hh"
#include "Symbol.hh"
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/utils/Array.hh>

namespace semantic {

    class IModule  {

	Namespace _space;
	std::vector <Namespace> _opens;
	std::vector <Namespace> publicOpens;
	Frame _constructor;
	Frame _destructor;
	
	Scope globalScope;
	bool _isAuto = true;
	
    public:

	IModule (const Namespace&);

	Symbol get (std::string name);
	
	Symbol getFor (std::string name, const Namespace & space);

	Symbol getAlikeFor (std::string name, const Namespace & space);

	std::vector <Symbol> getAll (std::string name);

	std::vector <Symbol> getAllFor (std::string name, const Namespace & space);

	void insert (Symbol sym);

	void addOpen (const Namespace& space);

	void close (const Namespace& space);

	void addPublicOpen (const Namespace& space);

	std::vector <Namespace> opens ();

	std::vector <Namespace> accessible ();

	bool authorized (const Namespace&);

	Frame & constructor ();

	Frame & destructor ();
	
	const Namespace& space ();

	bool & isAuto ();
	
    private:

	std::vector <Namespace> accessible (std::vector <Namespace>&);
	
    };

    typedef IModule* Module;
    
}
