#include <ymir/semantic/generator/value/UniqValue.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {
    namespace generator {
	
	UniqValue::UniqValue () :
	    Value (lexing::Word::eof (), Generator::empty ()),
	    _value (Generator::empty ())
	{}

	UniqValue::UniqValue (const lexing::Word & location, const Generator & type, const Generator & value, uint refId) :
	    Value (location, type),
	    _value (value),
	    _refId (refId)	    
	{}

	Generator UniqValue::init (const lexing::Word & location, const Generator & type, const Generator & value) {
	    return Generator {new (Z0) UniqValue (location, type, value, Generator::getLastId ())};
	}

	Generator UniqValue::clone () const {
	    return Generator {new (Z0) UniqValue (*this)};
	}
			
	bool UniqValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    UniqValue thisUniqValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisUniqValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool UniqValue::equals (const Generator & other) const {
	    if (!other.is <UniqValue> ()) return false;
	    else 
		return this-> _value.equals (other.to<UniqValue> ()._value);		    
	}


	const Generator & UniqValue::getValue () const {
	    return this-> _value;
	}

	uint UniqValue::getRefId () const {
	    return this-> _refId;
	}
	
	std::string UniqValue::prettyString () const {
	    return this-> _value.prettyString ();
	}
	
    }
}
