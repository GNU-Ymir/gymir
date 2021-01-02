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

	bool NullValue::equals (const Generator & gen) const {
	    if (!gen.is <NullValue> ()) return false;
	    return true;
	}

	std::string NullValue::prettyString () const {
	    return "null";
	}
	
    }
    
}
