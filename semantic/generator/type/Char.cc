#include <ymir/semantic/generator/type/Char.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	// c16 are currently not supported TODO
	std::vector <std::string> Char::NAMES = {"c8", "c32", "char"};
	
	uint32_t Char::INIT = 0;
	
	Char::Char () :
	    Type ()		  
	{}

	Char::Char (const lexing::Word & loc, int32_t size) :
	    Type (loc, loc.getStr ()),
	    _size (size)
	{}

	Generator Char::init (const lexing::Word & loc, int32_t size) {
	    return Generator {new (NO_GC) Char (loc, size)};
	}

	Generator Char::clone () const {
	    return Generator {new (NO_GC) Char (*this)};
	}
		
	bool Char::equals (const Generator & gen) const {
	    if (!gen.is <Char> ()) return false;
	    auto integer = gen.to<Char> ();
	    if (this-> _size == integer._size) return true;
	    else {
		return (this-> _size == 0 && integer._size == 32) ||
		       (this-> _size == 32 && integer._size == 0);
	    }
	}

	std::string Char::typeName () const {
	    if (this-> _size > 0)
		return Ymir::OutBuffer ("c", this-> _size).str ();
	    return "char";
	}	
    
	int32_t Char::getSize () const {
	    return this-> _size;
	}
	
    }
}
