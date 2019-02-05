#include <ymir/semantic/generator/type/Array.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::string Array::NAME = "[";
	
	Array::Array () :
	    Type (),
	    _inner (Generator::empty ()),
	    _size (false)
	{
	    this-> isComplex (true);
	}

	Array::Array (const lexing::Word & loc, const Generator & inner, int size) :
	    Type (loc, loc.str),
	    _inner (inner),
	    _size (size)
	{
	    this-> isComplex (true);
	}

	Generator Array::init (const lexing::Word & loc, const Generator & inner, int size) {
	    return Generator {new (Z0) Array (loc, inner, size)};
	}

	Generator Array::clone () const {
	    return Generator {new (Z0) Array (*this)};
	}
		
	bool Array::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Array thisArray; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisArray) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool Array::equals (const Generator & gen) const {
	    if (!gen.is<Array> ()) return false;
	    auto array = gen.to <Array> ();
	    return this-> _inner.equals (array._inner) && this-> _size == array._size;
	}

	std::string Array::typeName () const {
	    Ymir::OutBuffer buf;
	    if (this-> _size != -1) 
		buf.write ("[", this-> _inner.to<Type> ().typeName (), " ; ", this-> _size, "]");
	    else
		buf.write ("[", this-> _inner.to <Type> ().typeName (), "]");	    
	    return buf.str ();
	}	

	const Generator & Array::getInner () const {
	    return this-> _inner;
	}

	int Array::size () const {
	    return this-> _size;
	}

	bool Array::isStatic () const {
	    return this-> _size != -1;
	}
	
    }
}
