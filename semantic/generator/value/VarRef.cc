#include <ymir/semantic/generator/value/VarRef.hh>

namespace semantic {
    namespace generator {

	VarRef::VarRef () :
	    Value (lexing::Word::eof (), Generator::empty ())
	{}

	VarRef::VarRef (const lexing::Word & location, const std::string & name, const Generator & type, uint id) :
	    Value (location, type),
	    _name (name),
	    _refId (id)
	{}

	Generator VarRef::init (const lexing::Word & location, const std::string & name, const Generator & type, uint id) {
	    return Generator {new (Z0) VarRef (location, name, type, id)};
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

    }
}
