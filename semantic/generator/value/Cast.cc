#include <ymir/semantic/generator/value/Cast.hh>

namespace semantic {

    namespace generator {

	Cast::Cast () :
	    Value (),
	    _who (Generator::empty ())
	{}

	Cast::Cast (const lexing::Word & loc, const Generator & type, const Generator & who) :
	    Value (loc, type),
	    _who (who)
	{
	    this-> setThrowers (this-> _who.getThrowers ());
	}
	
	Generator Cast::init (const lexing::Word & loc, const Generator & type, const Generator & who) {
	    return Generator {new Cast (loc, type, who)};
	}
    
	Generator Cast::clone () const {
	    return Generator {new (Z0) Cast (*this)};
	}

	bool Cast::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Cast thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Cast::equals (const Generator & gen) const {
	    if (!gen.is <Cast> ()) return false;
	    auto bin = gen.to<Cast> ();	    
	    return this-> _who.equals (bin._who);
	}

	const Generator & Cast::getWho () const {
	    return this-> _who;
	}

	std::string Cast::prettyString () const {
	    return Ymir::format ("cast!(%) (%)", this-> getType ().to <Type> ().getTypeName (), this-> _who.prettyString ());
	}
    }
    
}
