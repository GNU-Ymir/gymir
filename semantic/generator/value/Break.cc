#include <ymir/semantic/generator/value/Break.hh>

namespace semantic {

    namespace generator {

	Break::Break () :
	    Value (),
	    _value (Generator::empty ())
	{
	    this-> setBreaker (true);
	}

	Break::Break (const lexing::Word & loc, const Generator & type, const Generator & value) :
	    Value (loc, type),
	    _value (value)
	{
	    this-> setBreaker (true);
	    this-> setThrowers (this-> _value.getThrowers ());
	}
	
	Generator Break::init (const lexing::Word & loc, const Generator & type, const Generator & value) {
	    return Generator {new (NO_GC) Break (loc, type, value)};
	}
    
	Generator Break::clone () const {
	    return Generator {new (NO_GC) Break (*this)};
	}

	bool Break::equals (const Generator & gen) const {
	    if (!gen.is <Break> ()) return false;
	    auto bin = gen.to<Break> ();	    
	    return this-> _value.equals (bin._value);
	}

	const Generator & Break::getValue () const {
	    return this-> _value;
	}

	std::string Break::prettyString () const {
	    return Ymir::format ("break ", this-> _value.prettyString ());
	}
	
    }
    
}
