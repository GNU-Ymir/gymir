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
	    return Generator {new (NO_GC) Referencer (loc, type, who)};
	}
    
	Generator Referencer::clone () const {
	    return Generator {new (NO_GC) Referencer (*this)};
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
