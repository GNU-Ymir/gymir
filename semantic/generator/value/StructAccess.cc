#include <ymir/semantic/generator/value/StructAccess.hh>

namespace semantic {

    namespace generator {

	StructAccess::StructAccess () :
	    _str (Generator::empty ())
	{
	    this-> isLvalue (true);
	}

	StructAccess::StructAccess (const lexing::Word & loc, const Generator & type, const Generator & str, const std::string & field) :
	    Value (loc, type),
	    _str (str),
	    _field (field)
	{
	    this-> isLvalue (true);
	}


	Generator StructAccess::init (const lexing::Word & loc, const Generator & type, const Generator & str, const std::string & field) {
	    return Generator {new StructAccess (loc, type, str, field)};
	}
	
	Generator StructAccess::clone () const {
	    return Generator {new StructAccess (*this)};
	}

	bool StructAccess::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    StructAccess thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}
	
	bool StructAccess::equals (const Generator & gen) const {
	    if (!gen.is <StructAccess> ()) return false;
	    auto bin = gen.to<StructAccess> ();	    
	    return bin._str.equals (this-> _str) && bin._field == this-> _field;
	}

	const Generator & StructAccess::getStruct () const {
	    return this-> _str;
	}

	const std::string & StructAccess::getField () const {
	    return this-> _field;
	}

	std::string StructAccess::prettyString () const {
	    return Ymir::format ("%.%", this-> _str.prettyString (), this-> _field);
	}

    }    

}