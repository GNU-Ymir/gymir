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
	    return Generator {new UnaryBool (loc, op, type, operand)};
	}

	Generator UnaryBool::clone () const {
	    return Generator {new (Z0) UnaryBool (*this)};
	}
	
	bool UnaryBool::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    UnaryBool thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Unary::isOf (type);	
	}

	
    }

}
