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


	FieldOffsetIndex::FieldOffsetIndex () :
	    _str (Generator::empty ())
	{
	    this-> isLvalue (true);
	}

	FieldOffsetIndex::FieldOffsetIndex (const lexing::Word & loc, const Generator & type, const Generator & str, ulong field) :
	    Value (loc, type),
	    _str (str),
	    _field (field)
	{
	    this-> isLvalue (true);	   	    
	    this-> setThrowers (str.getThrowers ());
	}


	Generator FieldOffsetIndex::init (const lexing::Word & loc, const Generator & type, const Generator & str, ulong field) {
	    return Generator {new (NO_GC) FieldOffsetIndex (loc, type, str, field)};
	}
	
	Generator FieldOffsetIndex::clone () const {
	    return Generator {new (NO_GC) FieldOffsetIndex (*this)};
	}
	
	bool FieldOffsetIndex::equals (const Generator & gen) const {
	    if (!gen.is <FieldOffsetIndex> ()) return false;
	    auto bin = gen.to<FieldOffsetIndex> ();	    
	    return bin._str.equals (this-> _str) && bin._field == this-> _field;
	}

	const Generator & FieldOffsetIndex::getTuple () const {
	    return this-> _str;
	}

	ulong FieldOffsetIndex::getField () const {
	    return this-> _field;
	}

	std::string FieldOffsetIndex::prettyString () const {
	    return Ymir::format ("%.%", this-> _str.prettyString (), this-> _field);
	}

	
    }    

}
