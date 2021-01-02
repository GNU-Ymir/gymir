#include <ymir/semantic/generator/value/Return.hh>

namespace semantic {

    namespace generator {

	Return::Return () :
	    Value (),
	    _frameType (Generator::empty ()),
	    _value (Generator::empty ())
	{
	    this-> setReturner (true);
	}

	Return::Return (const lexing::Word & loc, const Generator & type, const Generator & fun_type, const Generator & value) :
	    Value (loc, type),
	    _frameType (fun_type),
	    _value (value)
	{
	    this-> setReturner (true);
	    this-> setThrowers (this-> _value.getThrowers ());
	}
	
	Generator Return::init (const lexing::Word & loc, const Generator & type, const Generator & fun_type, const Generator & value) {
	    return Generator {new (NO_GC) Return (loc, type, fun_type, value)};
	}
    
	Generator Return::clone () const {
	    return Generator {new (NO_GC) Return (*this)};
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
