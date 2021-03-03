#include <ymir/semantic/generator/value/SliceConcat.hh>

namespace semantic {

    namespace generator {

	SliceConcat::SliceConcat () :
	    Value (),
	    _left (Generator::empty ()),
	    _right (Generator::empty ())
	{
	}

	SliceConcat::SliceConcat (const lexing::Word & loc, const Generator & type, const Generator & left, const Generator & right) :
	    Value (loc, type),
	    _left (left),
	    _right (right)
	{
	    auto vth = this-> _left.getThrowers ();
	    auto & ith = this-> _right.getThrowers ();
	    vth.insert (vth.end (), ith.begin (), ith.end ());
	    
	    this-> setThrowers (vth);
	}
	
	Generator SliceConcat::init (const lexing::Word & loc, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new (NO_GC) SliceConcat (loc, type, left, right)};
	}
    
	Generator SliceConcat::clone () const {
	    return Generator {new (NO_GC) SliceConcat (*this)};
	}

	bool SliceConcat::equals (const Generator & gen) const {
	    if (!gen.is <SliceConcat> ()) return false;
	    auto bin = gen.to<SliceConcat> ();	    
	    return bin._left.equals (this-> _left) && bin._right.equals (this-> _right);
	}

	const Generator & SliceConcat::getLeft () const {
	    return this-> _left;
	}

	const Generator & SliceConcat::getRight () const {
	    return this-> _right;
	}

	std::string SliceConcat::prettyString () const {
	    return Ymir::format ("% ~ %", this-> _left.prettyString (), this-> _right.prettyString ());
	}
    }
    
}
