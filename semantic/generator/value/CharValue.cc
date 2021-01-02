#include <ymir/semantic/generator/value/CharValue.hh>

namespace semantic {

    namespace generator {

	CharValue::CharValue () :
	    Value ()
	{}

	CharValue::CharValue (const lexing::Word & loc, const Generator & type, uint value) :
	    Value (loc, type),
	    _value (value)
	{}
       
	Generator CharValue::init (const lexing::Word & loc, const Generator & type, uint value) {
	    return Generator {new (NO_GC) CharValue (loc, type, value)};
	}

	Generator CharValue::clone () const {
	    return Generator {new (NO_GC) CharValue (*this)};
	}

	bool CharValue::equals (const Generator & gen) const {
	    if (!gen.is <CharValue> ()) return false;
	    auto floatValue = gen.to <CharValue> ();
	    return this-> getType ().equals (floatValue.getType ())
		&& this-> _value == floatValue._value;
	}

	uint CharValue::getValue () const {
	    return this-> _value;
	}

	std::string CharValue::prettyString () const {
	    return Ymir::format ("'%'", this-> _value);
	}
	
    }
    
}
