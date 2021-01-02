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
	    return Generator {new (NO_GC) Cast (loc, type, who)};
	}
    
	Generator Cast::clone () const {
	    return Generator {new (NO_GC) Cast (*this)};
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
