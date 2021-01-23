#include <ymir/semantic/generator/GlobalVar.hh>
#include <ymir/semantic/generator/value/VarDecl.hh>

namespace semantic {
    namespace generator {

	GlobalVar::GlobalVar () :
	    IGenerator (lexing::Word::eof (), ""),
	    _type (Generator::empty ()),
	    _value (Generator::empty ())
	{}

	GlobalVar::GlobalVar (const lexing::Word & location, const std::string & name, bool isMutable, const Generator & type, const Generator & value) :
	    IGenerator (location, name),	   
	    _type (type),
	    _value (value),
	    _isMutable (isMutable)
	{
	    this-> _varRefId = VarDecl::__lastId__;
	    VarDecl::__lastId__ += 1;
	}

	Generator GlobalVar::init (const lexing::Word & location, const std::string & name, bool isMutable, const Generator & type, const Generator & value) {
	    return Generator {new (NO_GC) GlobalVar (location, name, isMutable, type, value)};
	}

	Generator GlobalVar::clone () const {
	    return Generator {new (NO_GC) GlobalVar (*this)};
	}

	bool GlobalVar::isMutable () const {
	    return this-> _isMutable;
	}
	
	const Generator & GlobalVar::getType () const {
	    return this-> _type;
	}
	
	const Generator & GlobalVar::getValue () const {
	    return this-> _value;
	}
		
	uint GlobalVar::getUniqId () const {
	    return _varRefId;
	}

	bool GlobalVar::equals (const Generator & other) const {
	    if (!other.is <GlobalVar> ()) return false;
	    else 
		return other.getName () == this-> getName ();
	}

    }
}
