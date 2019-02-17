#include <ymir/semantic/generator/value/UnaryInt.hh>
#include <ymir/semantic/generator/type/Integer.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	UnaryInt::UnaryInt () :
	    Unary ()
	{}

	UnaryInt::UnaryInt (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) :
	    Unary (loc, op, type, operand)
	{
	    if (!operand.to<Value> ().getType ().is<Integer> ()) {
		Ymir::Error::halt ("%(r) Malformed UnaryInt, operand type is : %(y)", "Critical", operand.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    // All the operation we can do on ints produce int or bool
	    if (!type.is <Integer> ()) {
		Ymir::Error::halt ("%(r) Malformed UnaryInt, type is : %(y)", "Critical", type.to <Type> ().getTypeName ());
	    }	    
	}
	
	Generator UnaryInt::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) {
	    return Generator {new UnaryInt (loc, op, type, operand)};
	}

	Generator UnaryInt::clone () const {
	    return Generator {new (Z0) UnaryInt (*this)};
	}
	
	bool UnaryInt::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    UnaryInt thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Unary::isOf (type);	
	}

	
    }

}
