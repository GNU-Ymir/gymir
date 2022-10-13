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
	    return Generator {new (NO_GC) Fixed (loc, type, value)};
	}

	Generator Fixed::clone () const {
	    return Generator {new (NO_GC) Fixed (*this)};
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
	
	Generator ufixed (uint64_t value) {
	    Fixed::UI ui; ui.u = value;
	    auto loc = lexing::Word::eof ();
	    return Fixed::init (
		loc,
		Integer::init (loc, 0, false),
		ui 
	    );
	}

	Generator ifixed (int64_t value) {
	    Fixed::UI ui; ui.i = value;
	    auto loc = lexing::Word::eof ();
	    return Fixed::init (
		loc,
		Integer::init (loc, 0, true),
		ui
	    );
	}	

	Generator ufixed (const lexing::Word & loc, uint64_t value) {
	    Fixed::UI ui; ui.u = value;
	    return Fixed::init (
		loc,
		Integer::init (loc, 0, false),
		ui 
	    );
	}

	Generator ifixed (const lexing::Word & loc, int64_t value) {
	    Fixed::UI ui; ui.i = value;
	    return Fixed::init (
		loc,
		Integer::init (loc, 0, true),
		ui
	    );
	}	

    }

    
    
}
