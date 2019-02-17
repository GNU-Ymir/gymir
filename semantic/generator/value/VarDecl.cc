#include <ymir/semantic/generator/value/VarDecl.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {
    namespace generator {

	VarDecl::VarDecl () :
	    Value (lexing::Word::eof (), Generator::empty ()),
	    _name (""),
	    _type (Generator::empty ()),
	    _value (Generator::empty ()),
	    _isMutable (false)
	{}

	VarDecl::VarDecl (const lexing::Word & location, const std::string & name, const Generator & type, const Generator & value, bool isMutable) :
	    Value (location, Void::init (location)),
	    _name (name),
	    _type (type),
	    _value (value),
	    _isMutable (isMutable)
	{}

	Generator VarDecl::init (const lexing::Word & location, const std::string & name, const Generator & type, const Generator & value, bool isMutable) {
	    return Generator {new (Z0) VarDecl (location, name, type, value, isMutable)};
	}

	Generator VarDecl::clone () const {
	    return Generator {new (Z0) VarDecl (*this)};
	}
			
	bool VarDecl::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    VarDecl thisVarDecl; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisVarDecl) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool VarDecl::equals (const Generator & other) const {
	    if (!other.is <VarDecl> ()) return false;
	    else 
		return other.getName () == this-> getName () &&
		    this-> _type.equals (other.to<VarDecl> ()._type) &&
		    this-> _value.equals (other.to<VarDecl> ()._value)
		    ;
	}

	const Generator & VarDecl::getVarType () const {
	    return this-> _type;
	}

	const Generator & VarDecl::getVarValue () const {
	    return this-> _value;
	}

	bool VarDecl::isMutable () const {
	    return this-> _isMutable;
	}
	
    }
}
