#include <ymir/semantic/generator/value/UnaryFloat.hh>
#include <ymir/semantic/generator/type/Float.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	UnaryFloat::UnaryFloat () :
	    Unary ()
	{}

	UnaryFloat::UnaryFloat (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) :
	    Unary (loc, op, type, operand)
	{
	    if (!operand.to<Value> ().getType ().is<Float> ()) {
		Ymir::Error::halt ("%(r) Malformed UnaryFloat, operand type is : %(y)", "Critical", operand.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    // All the operation we can do on ints produce int or bool
	    if (!type.is <Float> ()) {
		Ymir::Error::halt ("%(r) Malformed UnaryFloat, type is : %(y)", "Critical", type.to <Type> ().getTypeName ());
	    }	    
	}
	
	Generator UnaryFloat::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) {
	    return Generator {new (NO_GC) UnaryFloat (loc, op, type, operand)};
	}

	Generator UnaryFloat::clone () const {
	    return Generator {new (NO_GC) UnaryFloat (*this)};
	}
	
	
    }

}
