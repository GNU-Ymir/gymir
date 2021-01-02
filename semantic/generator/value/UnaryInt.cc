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
	    return Generator {new (NO_GC) UnaryInt (loc, op, type, operand)};
	}

	Generator UnaryInt::clone () const {
	    return Generator {new (NO_GC) UnaryInt (*this)};
	}
	
	
    }

}
