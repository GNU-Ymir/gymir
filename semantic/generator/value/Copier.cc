#include <ymir/semantic/generator/value/Copier.hh>

namespace semantic {

    namespace generator {

	Copier::Copier () :
	    Value (),
	    _who (Generator::empty ())
	{}

	Copier::Copier (const lexing::Word & loc, const Generator & type, const Generator & who, bool isAny) :
	    Value (loc, type),
	    _who (who),
	    _isAny (isAny)
	{}
	
	Generator Copier::init (const lexing::Word & loc, const Generator & type, const Generator & who, bool isAny) {
	    return Generator {new Copier (loc, type, who, isAny)};
	}
    
	Generator Copier::clone () const {
	    return Generator {new (Z0) Copier (*this)};
	}

	bool Copier::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Copier thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Copier::equals (const Generator & gen) const {
	    if (!gen.is <Copier> ()) return false;
	    auto bin = gen.to<Copier> ();	    
	    return this-> _who.equals (bin._who);
	}

	const Generator & Copier::getWho () const {
	    return this-> _who;
	}

	bool Copier::isAny () const {
	    return this-> _isAny;
	}
	
	std::string Copier::prettyString () const {
	    return Ymir::format ("copy %", this-> _who.prettyString ());
	}
    }
    
}
