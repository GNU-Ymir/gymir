#include <ymir/semantic/generator/value/ParamVar.hh>

namespace semantic {
    namespace generator {

	ParamVar::ParamVar () :
	    Value (lexing::Word::eof (), Generator::empty ())
	{
	    this-> isLocal (false);
	}

	ParamVar::ParamVar (const lexing::Word & location, const Generator & type, bool isMutable, bool isSelf) :
	    Value (location, type),
	    _isMutable (isMutable),
	    _isSelf (isSelf)
	{
	    this-> isLocal (false);
	    this-> isLvalue (isMutable);
	}

	Generator ParamVar::init (const lexing::Word & location, const Generator & type, bool isMutable, bool isSelf) {
	    return Generator {new (NO_GC) ParamVar (location, type, isMutable, isSelf)};
	}

	Generator ParamVar::clone () const {
	    return Generator {new (NO_GC) ParamVar (*this)};
	}
			
	bool ParamVar::equals (const Generator & other) const {
	    if (!other.is <ParamVar> ()) return false;
	    else 
		return other.getName () == this-> getName () &&
		    this-> getType ().equals (other.to<ParamVar> ().getType ());
	}


	bool ParamVar::isMutable () const {
	    return this-> _isMutable;
	}

	bool ParamVar::isSelf () const {
	    return this-> _isSelf;
	}
	
	std::string ParamVar::prettyString () const {
	    if (this-> _isMutable) 
		return Ymir::format ("mut % : %", this-> getLocation ().getStr (), this-> getType ().prettyString ());
	    else
		return Ymir::format ("% : %", this-> getLocation ().getStr (), this-> getType ().prettyString ());
	}
    }
}
