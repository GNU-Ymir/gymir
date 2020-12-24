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

	bool CharValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    CharValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
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
	    return Ymir::format ("%", this-> _value);
	}
	
    }
    
}
