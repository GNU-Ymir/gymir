#include <ymir/semantic/generator/type/ErrorType.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::string ErrorType::NAME = "void";
	
	ErrorType::ErrorType () :
	    Type ()		  
	{}

	ErrorType::ErrorType (const lexing::Word & loc, const std::string & name, const std::list <Ymir::Error::ErrorMsg> & errors) :
	    Type (loc, loc.getStr ()),
	    _name (name),
	    _errors (errors)
	{}

	Generator ErrorType::init (const lexing::Word & loc, const std::string & name, const std::list <Ymir::Error::ErrorMsg> & errors) {
	    return Generator {new (NO_GC) ErrorType (loc, name, errors)};
	}

	Generator ErrorType::clone () const {
	    return Generator {new (NO_GC) ErrorType (*this)};
	}
		
	bool ErrorType::equals (const Generator & gen) const {
	    return gen.is<ErrorType> ();
	}

	std::string ErrorType::typeName () const {
	    return this-> _name;
	}	

	const std::list <Ymir::Error::ErrorMsg> & ErrorType::getErrors () const {
	    return _errors;
	}
	
    }
}
