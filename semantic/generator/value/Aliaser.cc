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
	{
	    this-> setThrowers (this-> _who.to <Value> ().getThrowers ());
	}
	
	Generator Aliaser::init (const lexing::Word & loc, const Generator & type, const Generator & who) {
	    return Generator {new (NO_GC) Aliaser (loc, type, who)};
	}
    
	Generator Aliaser::clone () const {
	    return Generator {new (NO_GC) Aliaser (*this)};
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
	    return this-> _who.prettyString ();
	}
	
    }
    
}
