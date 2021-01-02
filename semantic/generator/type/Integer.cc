#include <ymir/semantic/generator/type/Integer.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::vector <std::string> Integer::NAMES = {
	    "i8", "i16", "i32", "i64", "isize",
	    "u8", "u16", "u32", "u64", "usize"
	};

	long Integer::INIT = 0;

	std::string Integer::MIN_NAME = "min";

	std::string Integer::MAX_NAME = "max";

	std::string Integer::INIT_NAME = "init";
	
	Integer::Integer () :
	    Type ()		  
	{}

	Integer::Integer (const lexing::Word & loc, int size, bool isSigned) :
	    Type (loc, loc.getStr ()),
	    _size (size),
	    _isSigned (isSigned)
	{}

	Generator Integer::init (const lexing::Word & loc, int size, bool isSigned) {
	    return Generator {new (NO_GC) Integer (loc, size, isSigned)};
	}

	Generator Integer::clone () const {
	    return Generator {new (NO_GC) Integer (*this)};
	}
		
	bool Integer::equals (const Generator & gen) const {
	    if (!gen.is <Integer> ()) return false;
	    auto integer = gen.to<Integer> ();
	    return this-> _size == integer._size &&
		this-> _isSigned == integer._isSigned;
	}

	std::string Integer::typeName () const {
	    if (this-> _isSigned) {
		if (this-> _size > 0)
		    return Ymir::OutBuffer ("i", this-> _size).str ();
		return "isize";
	    } else {
		if (this-> _size > 0)
		    return Ymir::OutBuffer ("u", this-> _size).str ();
		return "usize";
	    }
	}	
    
	bool Integer::isSigned () const {
	    return this-> _isSigned;
	}

	int Integer::getSize () const {
	    return this-> _size;
	}
	
    }
}
