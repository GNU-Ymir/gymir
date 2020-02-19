#include <ymir/semantic/generator/value/Throw.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {

    namespace generator {

	Throw::Throw () :
	    Value (),
	    _value (Generator::empty ()),
	    _typeInfo (Generator::empty ())
	{
	    this-> isReturner (true);
	}

	Throw::Throw (const lexing::Word & loc, const Generator & typeInfo, const Generator & value) :
	    Value (loc, Void::init (loc)),
	    _value (value),
	    _typeInfo (typeInfo)
	{
	    this-> isReturner (true);
	    auto type = this-> _value.to <Value> ().getType ();
	    type.changeLocation (loc);
	    this-> setThrowers ({type});
	}
	
	Generator Throw::init (const lexing::Word & loc, const Generator & typeInfo, const Generator & value) {
	    return Generator {new Throw (loc,  typeInfo, value)};
	}
    
	Generator Throw::clone () const {
	    return Generator {new (Z0) Throw (*this)};
	}

	bool Throw::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Throw thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Throw::equals (const Generator & gen) const {
	    if (!gen.is <Throw> ()) return false;
	    auto bin = gen.to<Throw> ();	    
	    return this-> _value.equals (bin._value);
	}

	const Generator & Throw::getValue () const {
	    return this-> _value;
	}

	const Generator & Throw::getTypeInfo () const {
	    return this-> _typeInfo;
	}

	std::string Throw::prettyString () const {
	    return Ymir::format ("throw ", this-> _value.prettyString ());
	}
	
    }
    
}
