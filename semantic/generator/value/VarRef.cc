#include <ymir/semantic/generator/value/VarRef.hh>

namespace semantic {
    namespace generator {

	VarRef::VarRef () :
	    Value (lexing::Word::eof (), Generator::empty ()),
	    _name (""),
	    _refId (0),
	    _isMutable (false),
	    _value (Generator::empty ())
	{}

	VarRef::VarRef (const lexing::Word & location, const std::string & name, const Generator & type, uint id, bool isMutable, const Generator & value) :
	    Value (location, type),
	    _name (name),
	    _refId (id),
	    _isMutable (isMutable),
	    _value (value)
	{
	    this-> isLvalue (this-> _isMutable);
	}

	Generator VarRef::init (const lexing::Word & location, const std::string & name, const Generator & type, uint id, bool isMutable, const Generator & value) {
	    return Generator {new (Z0) VarRef (location, name, type, id, isMutable, value)};
	}

	Generator VarRef::clone () const {
	    return Generator {new (Z0) VarRef (*this)};
	}
			
	bool VarRef::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    VarRef thisVarRef; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisVarRef) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool VarRef::equals (const Generator & other) const {
	    if (!other.is <VarRef> ()) return false;
	    else 
		return other.getName () == this-> getName () &&
		    this-> getType ().equals (other.to<VarRef> ().getType ())
		    ;
	}

	uint VarRef::getRefId () const {
	    return this-> _refId;
	}

	const Generator & VarRef::getValue () const {
	    return this-> _value;
	}
	
	std::string VarRef::prettyString () const {
	    if (this-> _isMutable) 
		return Ymir::format ("mut %", this-> _name);
	    else
		return this-> _name;
	}
    }
}
