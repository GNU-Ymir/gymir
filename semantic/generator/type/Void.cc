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

	void Void::setMutable (bool) {}

	Generator Void::createMutable (bool) const {
	    return this-> clone ();
	}

	Generator Void::toMutable () const {
	    return this-> clone ();
	}

	Generator Void::toDeeplyMutable () const {
	    return this-> clone ();
	}
	
	std::string Void::typeName () const {
	    return "void";
	}	
	
    }
}
