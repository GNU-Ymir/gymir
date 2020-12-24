#include <ymir/semantic/generator/value/SliceValue.hh>

namespace semantic {

    namespace generator {

	SliceValue::SliceValue () :
	    Value (),
	    _ptr (Generator::empty ()),
	    _len (Generator::empty ())
	{}

	SliceValue::SliceValue (const lexing::Word & loc, const Generator & type, const Generator & ptr, const Generator & len) : 
	    Value (loc, type),
	    _ptr (ptr),
	    _len (len)
	{
	    // No throwable thing there i think, but to e sure :
	    auto pth = this-> _ptr.getThrowers ();
	    auto &lth = this-> _len.getThrowers ();
	    pth.insert (pth.end (), lth.begin (), lth.end ());
	    
	    this-> setThrowers (pth);
	}
       
	Generator SliceValue::init (const lexing::Word & loc, const Generator & type, const Generator & ptr, const Generator & len) {
	    return Generator {new (NO_GC) SliceValue (loc, type, ptr, len)};
	}

	Generator SliceValue::clone () const {
	    return Generator {new (NO_GC) SliceValue (*this)};
	}

	bool SliceValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    SliceValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool SliceValue::equals (const Generator & gen) const {
	    if (!gen.is <SliceValue> ()) return false;
	    auto floatValue = gen.to <SliceValue> ();
	    return this-> getType ().equals (floatValue.getType ())
		&& this-> _ptr.equals (floatValue._ptr) && this-> _len.equals (floatValue._len);
	}

	const Generator & SliceValue::getPtr () const {
	    return this-> _ptr;
	}

	const Generator & SliceValue::getLen () const {
	    return this-> _len;
	}
	
	std::string SliceValue::prettyString () const {
	    return Ymir::format ("{%, %}", this-> _ptr.prettyString (), this-> _len.prettyString ());
	}
	
    }
    
}
