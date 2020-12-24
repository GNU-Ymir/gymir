#include <ymir/semantic/generator/value/VtableAccess.hh>

namespace semantic {

    namespace generator {

	VtableAccess::VtableAccess () :
	    _str (Generator::empty ())
	{}

	VtableAccess::VtableAccess (const lexing::Word & loc, const Generator & type, const Generator & str, uint field) :
	    Value (loc, type),
	    _str (str),
	    _field (field)
	{
	    this-> setThrowers (this-> _str.getThrowers ());
	}

	Generator VtableAccess::init (const lexing::Word & loc, const Generator & type, const Generator & str, uint field) {
	    return Generator {new (NO_GC) VtableAccess (loc, type, str, field)};
	}
	
	Generator VtableAccess::clone () const {
	    return Generator {new (NO_GC) VtableAccess (*this)};
	}

	bool VtableAccess::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    VtableAccess thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}
	
	bool VtableAccess::equals (const Generator & gen) const {
	    if (!gen.is <VtableAccess> ()) return false;
	    auto bin = gen.to<VtableAccess> ();	    
	    return bin._str.equals (this-> _str) && bin._field == this-> _field;
	}

	const Generator & VtableAccess::getClass () const {
	    return this-> _str;
	}

	uint VtableAccess::getField () const {
	    return this-> _field;
	}

	std::string VtableAccess::prettyString () const {
	    return Ymir::format ("%.%", this-> _str.prettyString (), this-> _field);
	}

    }    

}
