#include <ymir/semantic/generator/value/ModuleAccess.hh>
#include <ymir/semantic/generator/type/NoneType.hh>

namespace semantic {

    namespace generator {

	ModuleAccess::ModuleAccess () :
	    Value ()
	{}

	ModuleAccess::ModuleAccess (const lexing::Word & loc, const Symbol & ref) :
	    Value (loc, NoneType::init (loc))
	{
	    auto aux = ref;
	    this-> _ref = aux.getPtr();
	}
	
	Generator ModuleAccess::init (const lexing::Word & loc, const Symbol & ref) {
	    return Generator {new (NO_GC) ModuleAccess (loc, ref)};
	}
    
	Generator ModuleAccess::clone () const {
	    return Generator {new (NO_GC) ModuleAccess (*this)};
	}

	bool ModuleAccess::equals (const Generator & gen) const {
	    if (!gen.is <ModuleAccess> ()) return false;
	    auto fr = gen.to<ModuleAccess> ();
	    return (Symbol {this-> _ref}).equals (fr._ref);
	}

	std::vector <Symbol> ModuleAccess::getLocal (const std::string & name) const {
	    return (Symbol {this-> _ref}).getLocal (name);
	}

	std::vector <Symbol> ModuleAccess::getLocalPublic (const std::string & name) const {
	    return (Symbol {this-> _ref}).getLocalPublic (name);
	}	
	
	Symbol ModuleAccess::getModRef () const {
	    return (Symbol {this-> _ref});
	}

	std::string ModuleAccess::prettyString () const {
	    return Ymir::format ("mod %", (Symbol {this-> _ref}).getRealName ());
	}
	
    }
    
}
