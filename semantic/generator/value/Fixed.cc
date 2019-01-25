#include <ymir/semantic/generator/value/Fixed.hh>

namespace semantic {

    namespace generator {

	Fixed::Fixed () :
	    Value (Generator::empty ())
	{}

	Fixed::Fixed (const Generator & type, UI value) :
	    Value (type),
	    _value (value)
	{}
       
	Generator Fixed::init (const Generator & type, UI value) {
	    return Generator {new (Z0) Fixed (type, value)};
	}

	Generator Fixed::clone () const {
	    return Generator {new (Z0) Fixed (*this)};
	}

	bool Fixed::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Fixed thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Fixed::equals (const Generator & gen) const {
	    if (!gen.is <Fixed> ()) return false;
	    auto fixed = gen.to <Fixed> ();
	    return this-> getType ().equals (fixed.getType ())
		&& (this-> _value.i == fixed._value.i ||
		    this-> _value.u == fixed._value.u);
	}

	Fixed::UI Fixed::getUI () const {
	    return this-> _value;
	}
	
    }
    
}
