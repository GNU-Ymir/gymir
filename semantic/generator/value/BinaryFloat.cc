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
		Ymir::Error::halt ("%(r) Malformed BinaryFloat, left operand type is : %(y)", "Critical", left.to<Value> ().getType ().to<Type> ().typeName ());
	    }

	    if (!right.to<Value> ().getType ().is <Float> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryFloat, right operand type is : %(y)", "Critical", right.to<Value> ().getType ().to<Type> ().typeName ());
	    }

	    // All the operation we can do on floats produce float or bool
	    if (!type.is <Float> () && !type.is <Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryFloat, type is : %(y)", "Critical", type.to <Type> ().typeName ());
	    }
	    
	}
	
	Generator BinaryFloat::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new BinaryFloat (loc, op, type, left, right)};
	}

	Generator BinaryFloat::clone () const {
	    return Generator {new (Z0) BinaryFloat (*this)};
	}
	
	bool BinaryFloat::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    BinaryFloat thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Binary::isOf (type);	
	}

	
    }

}
