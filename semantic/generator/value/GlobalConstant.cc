#include <ymir/semantic/generator/value/GlobalConstant.hh>

namespace semantic {

    namespace generator {

	GlobalConstant::GlobalConstant () :
	    Value (),
	    _name (""),
	    _value (Generator::empty ())
	{}

	GlobalConstant::GlobalConstant (const lexing::Word & loc, const std::string & name, const Generator & type, const Generator & value) :	    	    
	    Value (loc, type),
	    _name (name),
	    _value (value)
	{}
       
	Generator GlobalConstant::init (const lexing::Word & loc, const std::string & name, const Generator & type, const Generator & value) {	    	    
	    return Generator {new (NO_GC) GlobalConstant (loc, name, type, value)};
	}

	Generator GlobalConstant::clone () const {
	    return Generator {new (NO_GC) GlobalConstant (*this)};
	}

	bool GlobalConstant::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    GlobalConstant thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool GlobalConstant::equals (const Generator & gen) const {
	    if (!gen.is <GlobalConstant> ()) return false;
	    auto floatValue = gen.to <GlobalConstant> ();
	    return this-> getType ().equals (floatValue.getType ())
		&& this-> _value.equals (floatValue._value) && this-> _name == floatValue._name;
	}

	const Generator & GlobalConstant::getValue () const {
	    return this-> _value;
	}

	const std::string & GlobalConstant::getName () const {
	    return this-> _name;
	}

	std::string GlobalConstant::prettyString () const {
	    return Ymir::format ("% : %", this-> _name, this-> _value.prettyString ());
	}
	
    }
    
}
