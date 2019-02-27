#include <ymir/semantic/generator/value/ModuleAccess.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {

    namespace generator {

	ModuleAccess::ModuleAccess () :
	    Value (),
	    _ref (Symbol::empty ())
	{}

	ModuleAccess::ModuleAccess (const lexing::Word & loc, const Symbol & ref) :
	    Value (loc, Void::init (loc)),
	    _ref (ref)
	{}
	
	Generator ModuleAccess::init (const lexing::Word & loc, const Symbol & ref) {
	    return Generator {new ModuleAccess (loc, ref)};
	}
    
	Generator ModuleAccess::clone () const {
	    return Generator {new (Z0) ModuleAccess (*this)};
	}

	bool ModuleAccess::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ModuleAccess thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool ModuleAccess::equals (const Generator & gen) const {
	    if (!gen.is <ModuleAccess> ()) return false;
	    auto fr = gen.to<ModuleAccess> ();
	    return this-> _ref.equals (fr._ref);
	}

	std::vector <Symbol> ModuleAccess::getLocal (const std::string & name) const {
	    return this-> _ref.getLocal (name);
	}

	const Symbol & ModuleAccess::getModRef () const {
	    return this-> _ref;
	}
	
    }
    
}
