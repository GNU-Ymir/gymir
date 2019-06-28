#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::string NoneType::NAME = "void";
	
	NoneType::NoneType () :
	    Type ()		  
	{}

	NoneType::NoneType (const lexing::Word & loc) :
	    Type (loc, loc.str)
	{}

	Generator NoneType::init (const lexing::Word & loc) {
	    return Generator {new (Z0) NoneType (loc)};
	}

	Generator NoneType::clone () const {
	    return Generator {new (Z0) NoneType (*this)};
	}
		
	bool NoneType::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    NoneType thisNoneType; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisNoneType) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool NoneType::equals (const Generator & gen) const {
	    return gen.is<NoneType> ();
	}

	std::string NoneType::typeName () const {
	    return "none";
	}	
	
    }
}
