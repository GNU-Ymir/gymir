#include <ymir/semantic/generator/value/BinaryFloat.hh>
#include <ymir/semantic/generator/type/Float.hh>
#include <ymir/semantic/generator/type/Bool.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	BinaryFloat::BinaryFloat () :
	    Binary ()
	{}

	BinaryFloat::BinaryFloat (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) :
	    Binary (loc, op, type, left, right)
	{
	    if (!left.to<Value> ().getType ().is<Float> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryFloat, left operand type is : %(y)", "Critical", left.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    if (!right.to<Value> ().getType ().is <Float> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryFloat, right operand type is : %(y)", "Critical", right.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    // All the operation we can do on floats produce float or bool
	    if (!type.is <Float> () && !type.is <Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryFloat, type is : %(y)", "Critical", type.to <Type> ().getTypeName ());
	    }
	    
	}
	
	Generator BinaryFloat::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new (NO_GC) BinaryFloat (loc, op, type, left, right)};
	}

	Generator BinaryFloat::clone () const {
	    return Generator {new (NO_GC) BinaryFloat (*this)};
	}
	
	
    }

}
