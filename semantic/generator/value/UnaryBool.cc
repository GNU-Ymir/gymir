#include <ymir/semantic/generator/value/UnaryBool.hh>
#include <ymir/semantic/generator/type/Bool.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	UnaryBool::UnaryBool () :
	    Unary ()
	{}

	UnaryBool::UnaryBool (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) :
	    Unary (loc, op, type, operand)
	{
	    if (!operand.to<Value> ().getType ().is<Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed UnaryBool, operand type is : %(y)", "Critical", operand.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    // All the operation we can do on ints produce int or bool
	    if (!type.is <Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed UnaryBool, type is : %(y)", "Critical", type.to <Type> ().getTypeName ());
	    }	    
	}
	
	Generator UnaryBool::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) {
	    return Generator {new (NO_GC) UnaryBool (loc, op, type, operand)};
	}

	Generator UnaryBool::clone () const {
	    return Generator {new (NO_GC) UnaryBool (*this)};
	}
	
	
    }

}
