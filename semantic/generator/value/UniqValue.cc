#include <ymir/semantic/generator/value/UniqValue.hh>
#include <ymir/semantic/generator/value/VarDecl.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {
    namespace generator {
	
	UniqValue::UniqValue () :
	    Value (lexing::Word::eof (), Generator::empty ()),
	    _value (Generator::empty ())
	{}

	UniqValue::UniqValue (const lexing::Word & location, const Generator & type, const Generator & value, uint refId) :
	    Value (location, type),
	    _value (value),
	    _refId (refId)	    
	{
	    this-> isLvalue (true);
	    this-> setThrowers (this-> _value.getThrowers ());
	}
	
	Generator UniqValue::init (const lexing::Word & location, const Generator & type, const Generator & value) {
	    int id = VarDecl::__lastId__;
	    VarDecl::__lastId__ += 1;
	    return Generator {new (NO_GC) UniqValue (location, type, value, id)};
	}

	Generator UniqValue::clone () const {
	    return Generator {new (NO_GC) UniqValue (*this)};
	}
			
	bool UniqValue::equals (const Generator & other) const {
	    if (!other.is <UniqValue> ()) return false;
	    else 
		return this-> _value.equals (other.to<UniqValue> ()._value);		    
	}


	const Generator & UniqValue::getValue () const {
	    return this-> _value;
	}

	uint UniqValue::getRefId () const {
	    return this-> _refId;
	}
	
	std::string UniqValue::prettyString () const {
	    // Ymir::OutBuffer buf;
	    // buf.write ("Uniq:", this-> _refId, "(", this-> _value.prettyString (), ")");
	    // return buf.str ();
	    return this-> _value.prettyString ();
	}
	
    }
}
