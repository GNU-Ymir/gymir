#include <ymir/semantic/generator/type/Integer.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	Integer::Integer () :
	    Type ()		  
	{}

	Integer::Integer (const lexing::Word & loc, int size, bool isSigned) :
	    Type (loc, loc.str),
	    _size (size),
	    _isSigned (isSigned)
	{}

	Generator Integer::init (const lexing::Word & loc, int size, bool isSigned) {
	    return Generator {new (Z0) Integer (loc, size, isSigned)};
	}

	Generator Integer::clone () const {
	    return Generator {new (Z0) Integer (*this)};
	}
		
	bool Integer::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Integer thisInteger; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisInteger) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool Integer::equals (const Generator & gen) const {
	    if (!gen.is <Integer> ()) return false;
	    auto integer = gen.to<Integer> ();
	    return this-> _size == integer._size &&
		this-> _isSigned == integer._isSigned;
	}

	std::string Integer::typeName () const {
	    if (this-> _isSigned) {
		if (this-> _size >= 0)
		    return Ymir::OutBuffer ("u", this-> _size).str ();
		return "usize";
	    } else {
		if (this-> _size >= 0)
		    return Ymir::OutBuffer ("i", this-> _size).str ();
		return "isize";
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
