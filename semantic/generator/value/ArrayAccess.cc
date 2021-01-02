#include <ymir/semantic/generator/value/ArrayAccess.hh>

namespace semantic {

    namespace generator {

	ArrayAccess::ArrayAccess () :
	    Value (),
	    _array (Generator::empty ()),
	    _index (Generator::empty ())
	{
	    this-> isLvalue (true);
	}

	ArrayAccess::ArrayAccess (const lexing::Word & loc, const Generator & type, const Generator & array, const Generator & index) :
	    Value (loc, type),
	    _array (array),
	    _index (index)
	{
	    this-> isLvalue (true);
	    auto vth = this-> _array.getThrowers ();
	    auto & ith = this-> _index.getThrowers ();
	    vth.insert (vth.end (), ith.begin (), ith.end ());
	    
	    this-> setThrowers (vth);
	}
	
	Generator ArrayAccess::init (const lexing::Word & loc, const Generator & type, const Generator & array, const Generator & index) {
	    return Generator {new (NO_GC) ArrayAccess (loc, type, array, index)};
	}
    
	Generator ArrayAccess::clone () const {
	    return Generator {new (NO_GC) ArrayAccess (*this)};
	}

	bool ArrayAccess::equals (const Generator & gen) const {
	    if (!gen.is <ArrayAccess> ()) return false;
	    auto bin = gen.to<ArrayAccess> ();	    
	    return bin._array.equals (this-> _array) && bin._index.equals (this-> _index);
	}

	const Generator & ArrayAccess::getArray () const {
	    return this-> _array;
	}

	const Generator & ArrayAccess::getIndex () const {
	    return this-> _index;
	}

	std::string ArrayAccess::prettyString () const {
	    return Ymir::format ("% [%]", this-> _array.prettyString (), this-> _index.prettyString ());
	}
    }
    
}
