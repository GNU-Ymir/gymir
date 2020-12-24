#include <ymir/semantic/generator/value/None.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {

    namespace generator {

	None::None () :
	    Value ()
	{}

	None::None (const lexing::Word & loc) :
	    Value (loc, Void::init (loc))
	{}
       
	Generator None::init (const lexing::Word & loc) {
	    return Generator {new (NO_GC) None (loc)};
	}

	Generator None::clone () const {
	    return Generator {new (NO_GC) None (*this)};
	}

	bool None::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    None thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool None::equals (const Generator & gen) const {
	    return gen.is <None> ();
	    
	}
	
    }
    
}
