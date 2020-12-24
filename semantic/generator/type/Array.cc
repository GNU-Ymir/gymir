#include <ymir/semantic/generator/type/Array.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {
    namespace generator {

	std::string Array::LEN_NAME = "len";
	std::string Array::INNER_NAME = "inner";
	std::string Array::INIT = "init";
	
	Array::Array () :
	    Type (),
	    _size ()
	{
	    this-> isComplex (true);
	}

	Array::Array (const lexing::Word & loc, const Generator & inner, uint size) :
	    Type (loc, loc.str),
	    _size (size)
	{
	    this-> isComplex (true);
	    this-> setInners ({Type::init (inner.to<Type> (), inner.to <Type> ().isMutable (), false)});
	}

	Generator Array::init (const lexing::Word & loc, const Generator & inner, uint size) {
	    return Generator {new (NO_GC) Array (loc, inner, size)};
	}

	Generator Array::clone () const {
	    return Generator {new (NO_GC) Array (*this)};
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
	    return this-> getInners () [0].equals (array.getInners () [0]) && this-> _size == array._size;
	}

	std::string Array::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("[", this-> getInners () [0].to<Type> ().getTypeName (this-> isMutable ()), " ; ", (int) this-> _size, "]");
	    return buf.str ();
	}	

	uint Array::getSize () const {
	    return this-> _size;
	}
	
    }
}
