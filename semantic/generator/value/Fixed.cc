#include <ymir/semantic/generator/value/Fixed.hh>
#include <ymir/semantic/generator/type/Integer.hh>

namespace semantic {

    namespace generator {

	Fixed::Fixed () :
	    Value ()
	{}

	Fixed::Fixed (const lexing::Word & loc, const Generator & type, UI value) :
	    Value (loc, type),
	    _value (value)
	{}
       
	Generator Fixed::init (const lexing::Word & loc, const Generator & type, UI value) {
	    return Generator {new (Z0) Fixed (loc, type, value)};
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

	std::string Fixed::prettyString () const {
	    if (this-> getType ().to <Integer> ().isSigned ()) {
		return Ymir::format ("%", this-> _value.i);
	    } else
		return Ymir::format ("%", this-> _value.u);
	}
	
	Generator ufixed (ulong value) {
	    Fixed::UI ui; ui.u = value;
	    auto loc = lexing::Word::eof ();
	    return Fixed::init (
		loc,
		Integer::init (loc, 0, false),
		ui 
	    );
	}

	Generator ifixed (long value) {
	    Fixed::UI ui; ui.i = value;
	    auto loc = lexing::Word::eof ();
	    return Fixed::init (
		loc,
		Integer::init (loc, 0, true),
		ui
	    );
	}	

	Generator ufixed (const lexing::Word & loc, ulong value) {
	    Fixed::UI ui; ui.u = value;
	    return Fixed::init (
		loc,
		Integer::init (loc, 0, false),
		ui 
	    );
	}

	Generator ifixed (const lexing::Word & loc, long value) {
	    Fixed::UI ui; ui.i = value;
	    return Fixed::init (
		loc,
		Integer::init (loc, 0, true),
		ui
	    );
	}	

    }

    
    
}
