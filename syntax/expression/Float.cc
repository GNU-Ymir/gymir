#include <ymir/syntax/expression/Float.hh>

namespace syntax {

    Float::Float () {}

    Expression Float::init (const Float & alloc) {
	auto ret = new (Z0) Float ();
	ret-> _token = alloc._token;
	ret-> _token = alloc._decPart;
	return Expression {ret};
    }

    Expression Float::init (const lexing::Word & location, const lexing::Word & decPart) {
	auto ret = new (Z0) Float ();
	ret-> _token = location;
	ret-> _decPart = decPart;
	return Expression {ret};
    }

    Expression Float::clone () const {
	return Float::init (*this);
    }

    bool Float::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Float thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
