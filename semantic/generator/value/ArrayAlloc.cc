#include <ymir/semantic/generator/value/ArrayAlloc.hh>

namespace semantic {

    namespace generator {

	ArrayAlloc::ArrayAlloc () :
	    Value (),
	    _value (Generator::empty ()),
	    _size (Generator::empty ()),
	    _len (Generator::empty ()),
	    _staticLen (0),
	    _isDynamic (false)
	{
	}

	ArrayAlloc::ArrayAlloc (const lexing::Word & loc, const Generator & type, const Generator & value, const Generator & size, const Generator & len) :	
	    Value (loc, type),
	    _value (value),
	    _size (size),
	    _len (len),
	    _staticLen (0),
	    _isDynamic (true)
	{
	    auto vth = this-> _value.getThrowers ();
	    auto &sth = this-> _size.getThrowers ();
	    auto &lth = this-> _len.getThrowers ();
	    vth.insert (vth.end (), sth.begin (), sth.end ());
	    vth.insert (vth.end (), lth.begin (), lth.end ());
	    
	    this-> setThrowers (vth); 
	}


	ArrayAlloc::ArrayAlloc (const lexing::Word & loc, const Generator & type, const Generator & value, const Generator & size, ulong len) :
	    Value (loc, type),
	    _value (value),
	    _size (size),
	    _len (Generator::empty ()),
	    _staticLen (len),
	    _isDynamic (false)
	{
	    auto vth = this-> _value.getThrowers ();
	    auto &sth = this-> _size.getThrowers ();
	    vth.insert (vth.end (), sth.begin (), sth.end ());
	    
	    this-> setThrowers (vth);    
	}
		
	Generator ArrayAlloc::init (const lexing::Word & loc,  const Generator & type, const Generator & value, const Generator & size, const Generator & len) {
	    return Generator {new ArrayAlloc (loc, type, value, size, len)};
	}

	Generator ArrayAlloc::init (const lexing::Word & loc,  const Generator & type, const Generator & value, const Generator & size, ulong len) {
	    return Generator {new ArrayAlloc (loc, type, value, size, len)};
	}
    
	Generator ArrayAlloc::clone () const {
	    return Generator {new (Z0) ArrayAlloc (*this)};
	}

	bool ArrayAlloc::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ArrayAlloc thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool ArrayAlloc::equals (const Generator & gen) const {
	    if (!gen.is <ArrayAlloc> ()) return false;
	    auto bin = gen.to<ArrayAlloc> ();
	    if (this-> _isDynamic)
		return bin._value.equals (this-> _value) && bin._len.equals (this-> _len) && bin._size.equals (this-> _size);
	    else
		return bin._value.equals (this-> _value) && bin._staticLen == this-> _staticLen && bin._size.equals (this-> _size);
	}

	ulong ArrayAlloc::getStaticLen () const {
	    if (this-> _isDynamic) {
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    }
	    return this-> _staticLen;	    
	}


	bool ArrayAlloc::isDynamic () const {
	    return this-> _isDynamic;
	}


	const Generator & ArrayAlloc::getDynLen () const {
	    if (!this-> _isDynamic)
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return this-> _len;
	}

	const Generator & ArrayAlloc::getInnerTypeSize () const {
	    return this-> _size;
	}


	const Generator & ArrayAlloc::getDefaultValue () const {
	    return this-> _value;
	}

	
	std::string ArrayAlloc::prettyString () const {
	    if (this-> _isDynamic)
		return Ymir::format ("[% ; new %]", this-> _value.prettyString (), this-> _len.prettyString ());
	    else
		return Ymir::format ("[% ; %]", this-> _value.prettyString (), this-> _staticLen);
	}
    }
    
}
