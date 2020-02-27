#include <ymir/semantic/generator/value/Referencer.hh>

namespace semantic {

    namespace generator {

	Referencer::Referencer () :
	    Value (),
	    _who (Generator::empty ())
	{}

	Referencer::Referencer (const lexing::Word & loc, const Generator & type, const Generator & who) :
	    Value (loc, type),
	    _who (who)
	{
	    if (!type.to <Type> ().isRef ()) {
		Ymir::Error::halt ("%(r) Malformed Referencer, is not a reference type", "Critical");
	    }
	    this-> isLvalue (true);
	    this-> setThrowers (this-> _who.getThrowers ());
	}
	
	Generator Referencer::init (const lexing::Word & loc, const Generator & type, const Generator & who) {
	    return Generator {new (Z0) Referencer (loc, type, who)};
	}
    
	Generator Referencer::clone () const {
	    return Generator {new (Z0) Referencer (*this)};
	}

	bool Referencer::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Referencer thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Referencer::equals (const Generator & gen) const {
	    if (!gen.is <Referencer> ()) return false;
	    auto bin = gen.to<Referencer> ();	    
	    return this-> _who.equals (bin._who);
	}

	const Generator & Referencer::getWho () const {
	    return this-> _who;
	}

	std::string Referencer::prettyString () const {
	    return Ymir::format ("ref %", this-> _who.prettyString ());
	}
    }
    
}
