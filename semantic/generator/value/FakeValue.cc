#include <ymir/semantic/generator/value/FakeValue.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {

    namespace generator {

	FakeValue::FakeValue () :
	    Value ()
	{}

	FakeValue::FakeValue (const lexing::Word & loc, const Generator & type) :
	    Value (loc, type)
	{}
       
	Generator FakeValue::init (const lexing::Word & loc, const generator::Generator & type) {
	    return Generator {new (NO_GC) FakeValue (loc, type)};
	}

	Generator FakeValue::clone () const {
	    return Generator {new (NO_GC) FakeValue (*this)};
	}

	bool FakeValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    FakeValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool FakeValue::equals (const Generator & gen) const {
	    if (!gen.is <FakeValue> ()) return false;
	    return gen.to <FakeValue> ().getType ().equals (this-> getType ());	    
	}
	
    }
    
}
