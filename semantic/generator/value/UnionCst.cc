#include <ymir/semantic/generator/value/UnionCst.hh>

namespace semantic {

    namespace generator {

	UnionCst::UnionCst () :
	    Value (),
	    _str (Generator::empty ()),
	    _fieldName (""),
	    _typeCst (Generator::empty ()),
	    _param (Generator::empty ())
	{
	    this-> isLvalue (true);
	}

	UnionCst::UnionCst (const lexing::Word & loc, const Generator & type, const Generator & str, const std::string & name, const Generator & typeCst, const Generator & param) :
	    Value (loc, type),
	    _str (str),
	    _fieldName (name),
	    _typeCst (typeCst),
	    _param (param)
	{
	    this-> isLvalue (true);
	    this-> setThrowers (this-> _param.getThrowers ());
	}
	
	Generator UnionCst::init (const lexing::Word & loc, const Generator & type, const Generator & str, const std::string & name, const Generator & typeCst, const Generator & param) {
	    return Generator {new (NO_GC) UnionCst (loc, type, str, name, typeCst, param)};
	}
    
	Generator UnionCst::clone () const {
	    return Generator {new (NO_GC) UnionCst (*this)};
	}

	bool UnionCst::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    UnionCst thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool UnionCst::equals (const Generator & gen) const {
	    if (!gen.is <UnionCst> ()) return false;
	    auto call = gen.to <UnionCst> ();
	    if (!call.getStr ().equals (this-> _str)) return false;
	    if (!this-> _param.equals (call.getParameter ())) return false;
	    if (!this-> _typeCst.equals (call.getTypeCst ())) return false;	    

	    return true;
	}

	const Generator & UnionCst::getStr () const {
	    return this-> _str;
	}

	const std::string & UnionCst::getFieldName () const {
	    return this-> _fieldName;
	}
	
	const Generator & UnionCst::getTypeCst () const {
	    return this-> _typeCst;
	}
	
	const Generator & UnionCst::getParameter () const {
	    return this-> _param;
	}

	std::string UnionCst::prettyString () const {
	    return Ymir::format ("% (%-> %)", this-> _str.prettyString (), this-> _fieldName, this-> _param.prettyString ());
	}
	
    }
    
}
