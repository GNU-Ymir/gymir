#include <ymir/semantic/generator/value/StringValue.hh>

namespace semantic {

    namespace generator {

	StringValue::StringValue () :
	    Value ()
	{}

	StringValue::StringValue (const lexing::Word & loc, const Generator & type, const std::vector <char> &  value, ulong len) :
	    Value (loc, type),
	    _value (value),
	    _len (len)
	{}
       
	Generator StringValue::init (const lexing::Word & loc, const Generator & type, const std::vector<char> &  value, ulong len) {
	    return Generator {new (Z0) StringValue (loc, type, value, len)};
	}

	Generator StringValue::clone () const {
	    return Generator {new (Z0) StringValue (*this)};
	}

	bool StringValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    StringValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool StringValue::equals (const Generator & gen) const {
	    if (!gen.is <StringValue> ()) return false;
	    auto floatValue = gen.to <StringValue> ();
	    return this-> getType ().equals (floatValue.getType ())
		&& this-> _value == floatValue._value;
	}

	ulong StringValue::getLen  () const {
	    return this-> _len;
	}
	
	const std::vector<char> &  StringValue::getValue () const {
	    return this-> _value;
	}

	std::string StringValue::prettyString () const {
	    return Ymir::format ("_");
	}
	
    }
    
}
