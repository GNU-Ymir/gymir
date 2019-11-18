#include <ymir/semantic/generator/value/Return.hh>

namespace semantic {

    namespace generator {

	Return::Return () :
	    Value (),
	    _frameType (Generator::empty ()),
	    _value (Generator::empty ())
	{
	    this-> isReturner (true);
	}

	Return::Return (const lexing::Word & loc, const Generator & type, const Generator & fun_type, const Generator & value) :
	    Value (loc, type),
	    _frameType (fun_type),
	    _value (value)
	{
	    this-> isReturner (true);
	}
	
	Generator Return::init (const lexing::Word & loc, const Generator & type, const Generator & fun_type, const Generator & value) {
	    return Generator {new Return (loc, type, fun_type, value)};
	}
    
	Generator Return::clone () const {
	    return Generator {new (Z0) Return (*this)};
	}

	bool Return::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Return thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Return::equals (const Generator & gen) const {
	    if (!gen.is <Return> ()) return false;
	    auto bin = gen.to<Return> ();	    
	    return this-> _value.equals (bin._value);
	}

	const Generator & Return::getValue () const {
	    return this-> _value;
	}

	const Generator & Return::getFunType () const {
	    return this-> _frameType;
	}
	
	std::string Return::prettyString () const {
	    return Ymir::format ("return ", this-> _value.prettyString ());
	}
	
    }
    
}
