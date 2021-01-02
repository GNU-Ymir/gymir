#include <ymir/semantic/generator/value/BoolValue.hh>

namespace semantic {

    namespace generator {

	BoolValue::BoolValue () :
	    Value ()
	{}

	BoolValue::BoolValue (const lexing::Word & loc, const Generator & type, bool value) :
	    Value (loc, type),
	    _value (value)
	{}
       
	Generator BoolValue::init (const lexing::Word & loc, const Generator & type, bool value) {
	    return Generator {new (NO_GC) BoolValue (loc, type, value)};
	}

	Generator BoolValue::clone () const {
	    return Generator {new (NO_GC) BoolValue (*this)};
	}

	bool BoolValue::equals (const Generator & gen) const {
	    if (!gen.is <BoolValue> ()) return false;
	    return this-> _value == gen.to <BoolValue> ()._value;
	}

	bool BoolValue::getValue () const {
	    return this-> _value;
	}

	std::string BoolValue::prettyString () const {
	    if (this-> _value) return "true";
	    return "false";
	}
    }
    
}
