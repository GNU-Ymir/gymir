#include <ymir/semantic/generator/value/BinaryPtr.hh>
#include <ymir/semantic/generator/type/Integer.hh>
#include <ymir/semantic/generator/type/Pointer.hh>
#include <ymir/semantic/generator/type/Bool.hh>
#include <ymir/semantic/generator/type/ClassPtr.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	BinaryPtr::BinaryPtr () :
	    Binary ()
	{}

	BinaryPtr::BinaryPtr (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) :
	    Binary (loc, op, type, left, right)
	{
	    if (!left.to<Value> ().getType ().is<Pointer> () && !left.to <Value> ().getType ().is <ClassPtr> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryPtr, left operand type is : %(y)", "Critical", left.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    if (!right.to<Value> ().getType ().is <Integer> () && !right.to <Value> ().getType ().is <Pointer> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryPtr, right operand type is : %(y)", "Critical", right.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    // All the operation we can do on ints produce int or bool
	    if (!type.is <Integer> () && !type.is <Bool> () && !type.is <Pointer> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryPtr, type is : %(y)", "Critical", type.to <Type> ().getTypeName ());
	    }
	    
	}
	
	Generator BinaryPtr::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new (NO_GC) BinaryPtr (loc, op, type, left, right)};
	}

	Generator BinaryPtr::clone () const {
	    return Generator {new (NO_GC) BinaryPtr (*this)};
	}
	
	
    }

}
