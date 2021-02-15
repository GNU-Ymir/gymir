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
	    Type (loc, loc.getStr ())
	{}

	Generator Bool::init (const lexing::Word & loc) {
	    return Generator {new (NO_GC) Bool (loc)};
	}

	Generator Bool::clone () const {
	    return Generator {new (NO_GC) Bool (*this)};
	}
		
	bool Bool::directEquals (const Generator & gen) const {
	    return gen.is<Bool> ();
	}

	std::string Bool::typeName () const {
	    return Bool::NAME;
	}	
    	
    }
}
