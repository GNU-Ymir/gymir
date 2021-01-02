#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::string NoneType::NAME = "void";
	
	NoneType::NoneType () :
	    Type ()		  
	{}

	NoneType::NoneType (const lexing::Word & loc, const std::string & name) :
	    Type (loc, loc.getStr ()),
	    _name (name)
	{}

	Generator NoneType::init (const lexing::Word & loc, const std::string & name) {
	    return Generator {new (NO_GC) NoneType (loc, name)};
	}

	Generator NoneType::clone () const {
	    return Generator {new (NO_GC) NoneType (*this)};
	}
		
	bool NoneType::equals (const Generator & gen) const {
	    return gen.is<NoneType> ();
	}

	std::string NoneType::typeName () const {
	    return this-> _name;
	}	
	
    }
}
