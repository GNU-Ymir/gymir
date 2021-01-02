#include <ymir/semantic/generator/value/Throw.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {

    namespace generator {

	Throw::Throw () :
	    Value (),
	    _value (Generator::empty ()),
	    _typeInfo (Generator::empty ())
	{
	    this-> setReturner (true);
	}

	Throw::Throw (const lexing::Word & loc, const Generator & typeInfo, const Generator & value) :
	    Value (loc, Void::init (loc)),
	    _value (value),
	    _typeInfo (typeInfo)
	{
	    this-> setReturner (true);
	    this-> setThrowers ({Generator::init (loc, this-> _value.to <Value> ().getType ())});
	}
	
	Generator Throw::init (const lexing::Word & loc, const Generator & typeInfo, const Generator & value) {
	    return Generator {new (NO_GC) Throw (loc,  typeInfo, value)};
	}
    
	Generator Throw::clone () const {
	    return Generator {new (NO_GC) Throw (*this)};
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
