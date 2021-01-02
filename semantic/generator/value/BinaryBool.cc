#include <ymir/semantic/generator/value/BinaryBool.hh>
#include <ymir/semantic/generator/type/Integer.hh>
#include <ymir/semantic/generator/type/Bool.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	BinaryBool::BinaryBool () :
	    Binary ()
	{}

	BinaryBool::BinaryBool (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) :
	    Binary (loc, op, type, left, right)
	{
	    if (!left.to<Value> ().getType ().is<Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryBool, left operand type is : %(y)", "Critical", left.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    if (!right.to<Value> ().getType ().is <Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryBool, right operand type is : %(y)", "Critical", right.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    if (!type.is <Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryBool, type is : %(y)", "Critical", type.to <Type> ().getTypeName ());
	    }
	    
	}
	
	Generator BinaryBool::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new (NO_GC) BinaryBool (loc, op, type, left, right)};
	}

	Generator BinaryBool::clone () const {
	    return Generator {new (NO_GC) BinaryBool (*this)};
	}
	
	
    }

}
