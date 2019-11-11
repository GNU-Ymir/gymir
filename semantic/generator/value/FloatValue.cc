#include <ymir/semantic/generator/value/FloatValue.hh>

namespace semantic {

    namespace generator {

	FloatValue::FloatValue () :
	    Value ()
	{}

	FloatValue::FloatValue (const lexing::Word & loc, const Generator & type, const std::string & value) :
	    Value (loc, type),
	    _value (value),
	    _isStr (true)
	{}

	
	FloatValue::FloatValue (const lexing::Word & loc, const Generator & type, double value) :
	    Value (loc, type),
	    _value (""),
	    _valDouble (value),
	    _isStr (false)
	{}

	FloatValue::FloatValue (const lexing::Word & loc, const Generator & type, float value) :
	    Value (loc, type),
	    _value (""),
	    _valFloat (value),
	    _isStr (false)
	{}
		
	Generator FloatValue::init (const lexing::Word & loc, const Generator & type, const std::string & value) {
	    return Generator {new (Z0) FloatValue (loc, type, value)};
	}

	
	Generator FloatValue::init (const lexing::Word & loc, const Generator & type, float value) {
	    return Generator {new (Z0) FloatValue (loc, type, value)};
	}

		
	Generator FloatValue::init (const lexing::Word & loc, const Generator & type, double value) {
	    return Generator {new (Z0) FloatValue (loc, type, value)};
	}

	
	Generator FloatValue::clone () const {
	    return Generator {new (Z0) FloatValue (*this)};
	}

	bool FloatValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    FloatValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool FloatValue::equals (const Generator & gen) const {
	    if (!gen.is <FloatValue> ()) return false;
	    auto floatValue = gen.to <FloatValue> ();
	    return this-> getType ().equals (floatValue.getType ())
		&& this-> _value == floatValue._value;
	}

	const std::string & FloatValue::getValue () const {
	    return this-> _value;
	}

	bool FloatValue::isStr () const {
	    return this-> _isStr;
	}

	float FloatValue::getValueFloat () const {
	    return this-> _valFloat;
	}

	
	double FloatValue::getValueDouble () const {
	    return this-> _valDouble;
	}

	
	std::string FloatValue::prettyString () const {
	    return Ymir::format ("%", this-> _value);
	}
	
    }
    
}
