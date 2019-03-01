#include <ymir/semantic/generator/value/Aliaser.hh>

namespace semantic {

    namespace generator {

	Aliaser::Aliaser () :
	    Value (),
	    _who (Generator::empty ())
	{}

	Aliaser::Aliaser (const lexing::Word & loc, const Generator & type, const Generator & who) :
	    Value (loc, type),
	    _who (who)
	{}
	
	Generator Aliaser::init (const lexing::Word & loc, const Generator & type, const Generator & who) {
	    return Generator {new Aliaser (loc, type, who)};
	}
    
	Generator Aliaser::clone () const {
	    return Generator {new (Z0) Aliaser (*this)};
	}

	bool Aliaser::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Aliaser thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Aliaser::equals (const Generator & gen) const {
	    if (!gen.is <Aliaser> ()) return false;
	    auto bin = gen.to<Aliaser> ();	    
	    return this-> _who.equals (bin._who);
	}

	const Generator & Aliaser::getWho () const {
	    return this-> _who;
	}

	std::string Aliaser::prettyString () const {
	    return Ymir::format ("alias %", this-> _who.prettyString ());
	}
	
    }
    
}
