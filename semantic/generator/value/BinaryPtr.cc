#include <ymir/semantic/generator/value/BinaryPtr.hh>
#include <ymir/semantic/generator/type/Integer.hh>
#include <ymir/semantic/generator/type/Pointer.hh>
#include <ymir/semantic/generator/type/Bool.hh>
#include <ymir/semantic/generator/type/ClassPtr.hh>
#include <ymir/semantic/generator/type/ClassProxy.hh>
#include <ymir/semantic/generator/type/FuncPtr.hh>
#include <ymir/semantic/generator/type/Delegate.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	BinaryPtr::BinaryPtr () :
	    Binary ()
	{}

	BinaryPtr::BinaryPtr (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) :
	    Binary (loc, op, type, left, right)
	{
	    if (!left.to<Value> ().getType ().is<Pointer> () && !left.to <Value> ().getType ().is <ClassPtr> () && !left.to <Value> ().getType ().is <ClassProxy> () && !left.to<Value> ().getType ().is <FuncPtr> () && !left.to <Value> ().getType ().is <Delegate> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryPtr, left operand type is : %(y)", "Critical", left.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    if (!right.to<Value> ().getType ().is <Integer> () && !right.to <Value> ().getType ().is <Pointer> () && !right.to <Value> ().getType ().is <ClassPtr> () && !right.to <Value> ().getType ().is <ClassProxy> () && !right.to <Value> ().getType ().is <FuncPtr> () && !right.to <Value> ().getType ().is <Delegate> ()) {
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
