#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	Void::Void () :
	    Type ()		  
	{}

	Void::Void (const lexing::Word & loc) :
	    Type (loc, loc.str)
	{}

	Generator Void::init (const lexing::Word & loc) {
	    return Generator {new (Z0) Void (loc)};
	}

	Generator Void::clone () const {
	    return Generator {new (Z0) Void (*this)};
	}
		
	bool Void::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Void thisVoid; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisVoid) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool Void::equals (const Generator & gen) const {
	    return gen.is<Void> ();
	}

	std::string Void::typeName () const {
	    return "void";
	}	
	
    }
}
