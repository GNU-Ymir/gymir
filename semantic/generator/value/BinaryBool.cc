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
		Ymir::Error::halt ("%(r) Malformed BinaryBool, left operand type is : %(y)", "Critical", left.to<Value> ().getType ().to<Type> ().typeName ());
	    }

	    if (!right.to<Value> ().getType ().is <Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryBool, right operand type is : %(y)", "Critical", right.to<Value> ().getType ().to<Type> ().typeName ());
	    }

	    if (!type.is <Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryBool, type is : %(y)", "Critical", type.to <Type> ().typeName ());
	    }
	    
	}
	
	Generator BinaryBool::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new BinaryBool (loc, op, type, left, right)};
	}

	Generator BinaryBool::clone () const {
	    return Generator {new (Z0) BinaryBool (*this)};
	}
	
	bool BinaryBool::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    BinaryBool thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Binary::isOf (type);	
	}

	
    }

}
