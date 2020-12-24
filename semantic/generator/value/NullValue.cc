#include <ymir/semantic/generator/value/NullValue.hh>

namespace semantic {

    namespace generator {

	NullValue::NullValue () :
	    Value ()
	{}

	NullValue::NullValue (const lexing::Word & loc, const Generator & type) :
	    Value (loc, type)
	{}
       
	Generator NullValue::init (const lexing::Word & loc, const Generator & type) {
	    return Generator {new (NO_GC) NullValue (loc, type)};
	}

	Generator NullValue::clone () const {
	    return Generator {new (NO_GC) NullValue (*this)};
	}

	bool NullValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    NullValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool NullValue::equals (const Generator & gen) const {
	    if (!gen.is <NullValue> ()) return false;
	    return true;
	}

	std::string NullValue::prettyString () const {
	    return "null";
	}
	
    }
    
}
