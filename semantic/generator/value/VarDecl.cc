#include <ymir/semantic/generator/value/VarDecl.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {
    namespace generator {

	uint VarDecl::__lastId__ = 0;

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
	{
	    this-> _varRefId = __lastId__;
	    __lastId__ += 1;
	    
	    this-> setThrowers (this-> _value.getThrowers ());
	}

	Generator VarDecl::init (const lexing::Word & location, const std::string & name, const Generator & type, const Generator & value, bool isMutable) {
	    return Generator {new (NO_GC) VarDecl (location, name, type, value, isMutable)};
	}

	Generator VarDecl::clone () const {
	    return Generator {new (NO_GC) VarDecl (*this)};
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

	uint VarDecl::getUniqId () const {
	    return _varRefId;
	}
	
	std::string VarDecl::prettyString () const {
	    auto val = this-> _value.prettyString ();
	    auto type = this-> _type.prettyString ();
	    if (this-> _isMutable) {
		return Ymir::format ("let mut % : %%%", this-> _name, type, this-> _value.isEmpty () ? "" : " = ", val);
	    } else
		return Ymir::format ("let % : %%%", this-> _name, type, this-> _value.isEmpty () ? "" : " = ", val);
	}
	
    }
}
