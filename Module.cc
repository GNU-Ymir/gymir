#include <ymir/semantic/pack/Module.hh>
#include <algorithm>
#include <ymir/semantic/pack/Table.hh>

namespace semantic {

    IModule::IModule (Namespace space) :
	_space (space)
    {}

    Symbol IModule::get (std::string name) {
	return this-> globalScope.get (name);
    }

    std::vector <Symbol> IModule::getAll (std::string name) {
	return this-> globalScope.getAll (name);
    }

    void IModule::insert (Symbol symbol) {
	this-> globalScope.set (symbol-> sym.getStr (), symbol);
    }

    void IModule::addOpen (Namespace space) {
	this-> _opens.push_back (space);       
    }

    void IModule::close (Namespace space) {
	auto id = find (this-> _opens, space);
	if (id != this-> _opens.end ()) 
	    this-> _opens.erase (id);

	id = find (this-> publicOpens, space);
	if (id != this-> publicOpens.end ())
	    this-> publicOpens.erase (id);
	
    }
    
    void IModule::addPublicOpen (Namespace space) {
	this-> publicOpens.push_back (space);
    }

    std::vector <Namespace> IModule::opens () {
	return this-> _opens;
    }

    std::vector <Namespace> IModule::accessible () {
	return accessible ({this-> _space});
    }

    std::vector <Namespace> IModule::accessible (std::vector <Namespace> dones) {
	for (auto sp : this-> publicOpens) {
	    if (!canFindRef (dones, sp)) {
		dones.push_back (sp);
		auto mod = Table::instance ().getModule (sp);
		if (mod) {
		    auto access = mod-> accessible (dones);
		    dones.insert (dones.end (), access.begin (), access.end ());
		}
	    }
	}
	return dones;
    }

    bool IModule::authorized (Namespace space) {
	if (this-> _space.isSubOf (space)) return true;
	for (auto it : this-> _opens) {
	    if (it.isSubOf (space)) return true;
	}

	for (auto it : this-> publicOpens) {
	    if (it.isSubOf (space)) return true;
	}
	
	return false;
    }

    Namespace IModule::space () {
	return this-> _space;
    }
    
}
