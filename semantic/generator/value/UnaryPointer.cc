#include <ymir/semantic/generator/value/UnaryPointer.hh>
#include <ymir/semantic/generator/type/Pointer.hh>
#include <ymir/semantic/generator/type/FuncPtr.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	UnaryPointer::UnaryPointer () :
	    Unary ()
	{
	    this-> isLvalue (true);
	}

	UnaryPointer::UnaryPointer (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) :
	    Unary (loc, op, type, operand)
	{
	    if (!operand.to<Value> ().getType ().is<Pointer> ()) {
		Ymir::Error::halt ("%(r) Malformed UnaryPointer, operand type is : %(y)", "Critical", operand.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    this-> isLvalue (true);
	}
	
	Generator UnaryPointer::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand) {
	    return Generator {new (NO_GC) UnaryPointer (loc, op, type, operand)};
	}

	Generator UnaryPointer::clone () const {
	    return Generator {new (NO_GC) UnaryPointer (*this)};
	}
	
	
    }

}
