#include <ymir/semantic/generator/type/Bool.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::string Bool::NAME = "bool";

	bool Bool::INIT = false;
	
	Bool::Bool () :
	    Type ()		  
	{}

	Bool::Bool (const lexing::Word & loc) :
	    Type (loc, loc.str)
	{}

	Generator Bool::init (const lexing::Word & loc) {
	    return Generator {new (Z0) Bool (loc)};
	}

	Generator Bool::clone () const {
	    return Generator {new (Z0) Bool (*this)};
	}
		
	bool Bool::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Bool thisBool; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisBool) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool Bool::equals (const Generator & gen) const {
	    return gen.is<Bool> ();
	}

	std::string Bool::typeName () const {
	    return Bool::NAME;
	}	
    	
    }
}
