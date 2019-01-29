#include <ymir/semantic/generator/value/ParamVar.hh>

namespace semantic {
    namespace generator {

	ParamVar::ParamVar () :
	    Value (lexing::Word::eof (), Generator::empty ())
	{}

	ParamVar::ParamVar (const lexing::Word & location, const std::string & name, const Generator & type) :
	    Value (location, name, type),
	    _name (name)
	{}

	Generator ParamVar::init (const lexing::Word & location, const std::string & name, const Generator & type) {
	    return Generator {new (Z0) ParamVar (location, name, type)};
	}

	Generator ParamVar::clone () const {
	    return Generator {new (Z0) ParamVar (*this)};
	}
			
	bool ParamVar::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ParamVar thisParamVar; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisParamVar) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool ParamVar::equals (const Generator & other) const {
	    if (!other.is <ParamVar> ()) return false;
	    else 
		return other.getName () == this-> getName () &&
		    this-> getType ().equals (other.to<ParamVar> ().getType ());
	}

    }
}
