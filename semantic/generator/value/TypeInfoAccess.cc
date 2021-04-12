#include <ymir/semantic/generator/value/TypeInfoAccess.hh>

namespace semantic {

    namespace generator {

	TypeInfoAccess::TypeInfoAccess () :
	    Value (),
	    _classRef (Generator::empty ())
	{}

	TypeInfoAccess::TypeInfoAccess (const lexing::Word & loc, const Generator & type, const Generator & ref) :
	    Value (loc, type),
	    _classRef (ref)
	{}
	
	Generator TypeInfoAccess::init (const lexing::Word & loc, const Generator & type, const Generator & classRef) {
	    return Generator {new (NO_GC) TypeInfoAccess (loc, type, classRef)};
	}
	
	Generator TypeInfoAccess::clone () const {
	    return Generator {new (NO_GC) TypeInfoAccess (*this)};
	}

	bool TypeInfoAccess::equals (const Generator & gen) const {
	    if (!gen.is <TypeInfoAccess> ()) return false;
	    auto call = gen.to <TypeInfoAccess> ();
	    return call.getClassRef ().equals (this-> _classRef);
	}

	const Generator & TypeInfoAccess::getClassRef () const {
	    return this-> _classRef;
	}
	
	std::string TypeInfoAccess::prettyString () const {
	    return Ymir::format ("typeinfo (%)", this-> _classRef.prettyString ());
	}
	
    }
    
}
