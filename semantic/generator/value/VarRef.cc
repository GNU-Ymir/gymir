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

	VarRef::VarRef (const lexing::Word & location, const std::string & name, const Generator & type, uint id, bool isMutable, const Generator & value, bool isSelf) :
	    Value (location, type),
	    _name (name),
	    _refId (id),
	    _isMutable (isMutable),
	    _value (value),
	    _isSelf (isSelf)
	{
	    this-> isLvalue (true);
	}

	Generator VarRef::init (const lexing::Word & location, const std::string & name, const Generator & type, uint id, bool isMutable, const Generator & value, bool isSelf) {
	    return Generator {new (NO_GC) VarRef (location, name, type, id, isMutable, value, isSelf)};
	}

	Generator VarRef::clone () const {
	    return Generator {new (NO_GC) VarRef (*this)};
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

	bool VarRef::isSelf () const {
	    return this-> _isSelf;
	}
	
	std::string VarRef::prettyString () const {
	    if (this-> _isMutable) 
		return Ymir::format ("mut %", this-> _name);
	    else
		return this-> _name;
	}
    }
}
