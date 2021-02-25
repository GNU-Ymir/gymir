#include <ymir/semantic/generator/value/FieldOffset.hh>

namespace semantic {

    namespace generator {

	FieldOffset::FieldOffset () :
	    _str (Generator::empty ())
	{
	    this-> isLvalue (true);
	}

	FieldOffset::FieldOffset (const lexing::Word & loc, const Generator & type, const Generator & str, const std::string & field) :
	    Value (loc, type),
	    _str (str),
	    _field (field)
	{
	    this-> isLvalue (true);	   	    
	    this-> setThrowers (str.getThrowers ());
	}


	Generator FieldOffset::init (const lexing::Word & loc, const Generator & type, const Generator & str, const std::string & field) {
	    return Generator {new (NO_GC) FieldOffset (loc, type, str, field)};
	}
	
	Generator FieldOffset::clone () const {
	    return Generator {new (NO_GC) FieldOffset (*this)};
	}
	
	bool FieldOffset::equals (const Generator & gen) const {
	    if (!gen.is <FieldOffset> ()) return false;
	    auto bin = gen.to<FieldOffset> ();	    
	    return bin._str.equals (this-> _str) && bin._field == this-> _field;
	}

	const Generator & FieldOffset::getStruct () const {
	    return this-> _str;
	}

	const std::string & FieldOffset::getField () const {
	    return this-> _field;
	}

	std::string FieldOffset::prettyString () const {
	    return Ymir::format ("%.%", this-> _str.prettyString (), this-> _field);
	}

    }    

}
