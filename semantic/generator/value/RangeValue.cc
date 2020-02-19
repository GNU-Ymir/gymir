#include <ymir/semantic/generator/value/RangeValue.hh>

namespace semantic {

    namespace generator {

	RangeValue::RangeValue () :
	    Value (),
	    _left (Generator::empty ()),
	    _right (Generator::empty ()),
	    _step (Generator::empty ()),
	    _isFull (Generator::empty ())
	{}

	RangeValue::RangeValue (const lexing::Word & loc, const Generator & type, const Generator & left, const Generator & right, const Generator & step, const Generator & full) :
	    Value (loc, type),
	    _left (left),
	    _right (right),
	    _step (step),
	    _isFull (full)
	{
	    auto lthrs = this-> _left.getThrowers ();
	    auto &rthrs = this-> _right.getThrowers ();
	    lthrs.insert (lthrs.end (), rthrs.begin (), rthrs.end ());
	    
	    this-> setThrowers (lthrs);
	}
	
	Generator RangeValue::init (const lexing::Word & loc, const Generator & type, const Generator & left, const Generator & right, const Generator & step, const Generator & full) {
	    return Generator {new RangeValue (loc, type, left, right, step, full)};
	}
    
	Generator RangeValue::clone () const {
	    return Generator {new (Z0) RangeValue (*this)};
	}

	bool RangeValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    RangeValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool RangeValue::equals (const Generator & gen) const {
	    if (!gen.is <RangeValue> ()) return false;
	    auto bin = gen.to<RangeValue> ();	    
	    if (!this-> _left.equals (bin._left)) return false;
	    if (!this-> _right.equals (bin._right)) return false;
	    if (!this-> _step.equals (bin._step)) return false;
	    if (!this-> _isFull.equals (bin._isFull)) return false;

	    return true;
	}

	const Generator & RangeValue::getLeft () const {
	    return this-> _left;
	}

	const Generator & RangeValue::getRight () const  {
	    return this-> _right;
	}

	const Generator & RangeValue::getStep () const {
	    return this-> _step;
	}

	const Generator & RangeValue::getIsFull () const {
	    return this-> _isFull;
	}

	std::string RangeValue::prettyString () const {
	    return Ymir::format ("r!(% .. %, %, %)", this-> _left.prettyString (), this-> _right.prettyString (), this-> _step.prettyString (), this-> _isFull.prettyString ());
	}
    }
    
}
