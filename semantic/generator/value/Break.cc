#include <ymir/semantic/generator/value/Break.hh>

namespace semantic {

    namespace generator {

	Break::Break () :
	    Value (),
	    _value (Generator::empty ())
	{
	    this-> isBreaker (true);
	}

	Break::Break (const lexing::Word & loc, const Generator & type, const Generator & value) :
	    Value (loc, type),
	    _value (value)
	{
	    this-> isBreaker (true);
	}
	
	Generator Break::init (const lexing::Word & loc, const Generator & type, const Generator & value) {
	    return Generator {new Break (loc, type, value)};
	}
    
	Generator Break::clone () const {
	    return Generator {new (Z0) Break (*this)};
	}

	bool Break::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Break thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
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
