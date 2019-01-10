#include <ymir/syntax/expression/Bool.hh>

namespace syntax {

    Bool::Bool () {}

    Expression Bool::init (const Bool & alloc) {
	auto ret = new (Z0) Bool ();
	ret-> _token = alloc._token;
	return Expression {ret};
    }

    Expression Bool::init (const lexing::Word & location) {
	auto ret = new (Z0) Bool ();
	ret-> _token = location;
	return Expression {ret};
    }

    Expression Bool::clone () const {
	return Bool::init (*this);
    }

    bool Bool::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Bool thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
