#include <ymir/semantic/generator/value/BinaryChar.hh>
#include <ymir/semantic/generator/type/Char.hh>
#include <ymir/semantic/generator/type/Bool.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    namespace generator {
	
	BinaryChar::BinaryChar () :
	    Binary ()
	{}

	BinaryChar::BinaryChar (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) :
	    Binary (loc, op, type, left, right)
	{
	    if (!left.to<Value> ().getType ().is<Char> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryChar, left operand type is : %(y)", "Critical", left.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    if (!right.to<Value> ().getType ().is <Char> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryChar, right operand type is : %(y)", "Critical", right.to<Value> ().getType ().to<Type> ().getTypeName ());
	    }

	    // All the operation we can do on ints produce int or bool
	    if (!type.is <Char> () && !type.is <Bool> ()) {
		Ymir::Error::halt ("%(r) Malformed BinaryChar, type is : %(y)", "Critical", type.to <Type> ().getTypeName ());
	    }
	    
	}
	
	Generator BinaryChar::init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right) {
	    return Generator {new (NO_GC) BinaryChar (loc, op, type, left, right)};
	}

	Generator BinaryChar::clone () const {
	    return Generator {new (NO_GC) BinaryChar (*this)};
	}
	
	bool BinaryChar::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    BinaryChar thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Binary::isOf (type);	
	}

	
    }

}
