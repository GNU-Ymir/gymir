#include <ymir/semantic/generator/value/BinaryInt.hh>
#include <ymir/semantic/generator/type/Integer.hh>
#include <ymir/semantic/generator/type/Bool.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	BinaryInt::BinaryInt () :
	    Binary ()
	{}

	BinaryInt::BinaryInt (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) :
	    Binary (loc, op, type, left, right)
	{
	    if (!left.to<Value> ().getType ().is<Integer> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryInt, left operand type is : %(y)", "Critical", left.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    if (!right.to<Value> ().getType ().is <Integer> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryInt, right operand type is : %(y)", "Critical", right.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    // All the operation we can do on ints produce int or bool
	    if (!type.is <Integer> () && !type.is <Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryInt, type is : %(y)", "Critical", type.to <Type> ().getTypeName ());
	    }
	    
	}
	
	Generator BinaryInt::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new (Z0) BinaryInt (loc, op, type, left, right)};
	}

	Generator BinaryInt::clone () const {
	    return Generator {new (Z0) BinaryInt (*this)};
	}
	
	bool BinaryInt::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    BinaryInt thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Binary::isOf (type);	
	}

	
    }

}
