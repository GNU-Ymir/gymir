#include <ymir/semantic/generator/value/Conditional.hh>

namespace semantic {

    namespace generator {

	Conditional::Conditional () :
	    Value (),
	    _test (Generator::empty ()),
	    _content (Generator::empty ()),
	    _else (Generator::empty ())
	{}
	
	Conditional::Conditional (const lexing::Word & loc, const Generator & type, const Generator & test, const Generator & content, const Generator & else_) :
	    Value (loc, type),
	    _test (test),
	    _content (content),
	    _else (else_)				    
	{}
	
	Generator Conditional::init (const lexing::Word & loc, const Generator & type, const Generator & test, const Generator & content, const Generator & else_) {
	    return Generator {new Conditional (loc, type, test, content, else_)};
	}
    
	Generator Conditional::clone () const {
	    return Generator {new (Z0) Conditional (*this)};
	}

	bool Conditional::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Conditional thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Conditional::equals (const Generator & gen) const {
	    if (!gen.is <Conditional> ()) return false;
	    auto bin = gen.to<Conditional> ();	    
	    return this-> _test.equals (bin._test) &&
		this-> _content.equals (bin._content) &&
		this-> _else.equals (bin._else);
	}

	const Generator & Conditional::getTest () const {
	    return this-> _test;
	}

	const Generator & Conditional::getContent () const {
	    return this-> _content;
	}
	
	const Generator & Conditional::getElse () const {
	    return this-> _else;
	}
	
    }
    
}
