#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	Binary::Binary () :
	    Value (),
	    _left (Generator::empty ()),
	    _right (Generator::empty ())
	{}

	Binary::Binary (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) :
	    Value (loc, type),
	    _left (left),
	    _right (right),
	    _operator (op)	    
	{}
	
	Generator Binary::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new Binary (loc, op, type, left, right)};
	}
    
	Generator Binary::clone () const {
	    return Generator {new (Z0) Binary (*this)};
	}

	bool Binary::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Binary thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Binary::equals (const Generator & gen) const {
	    if (!gen.is <Binary> ()) return false;
	    auto bin = gen.to<Binary> ();	    
	    return this-> _left.equals (bin._left) &&
		this-> _right.equals (bin._right) &&
		this-> _operator == bin._operator;		
	}

	const Generator & Binary::getLeft () const {
	    return this-> _left;
	}

	const Generator & Binary::getRight () const {
	    return this-> _right;
	}

	Binary::Operator Binary::getOperator () const {
	    return this-> _operator;
	}
	
    }
    
}
