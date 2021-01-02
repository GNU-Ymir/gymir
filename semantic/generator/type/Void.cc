#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::string Void::NAME = "void";
	
	Void::Void () :
	    Type ()		  
	{}

	Void::Void (const lexing::Word & loc) :
	    Type (loc, loc.getStr ())
	{}

	Generator Void::init (const lexing::Word & loc) {
	    return Generator {new (NO_GC) Void (loc)};
	}

	Generator Void::clone () const {
	    return Generator {new (NO_GC) Void (*this)};
	}
		
	bool Void::equals (const Generator & gen) const {
	    return gen.is<Void> ();
	}

	std::string Void::typeName () const {
	    return "void";
	}	
	
    }
}
