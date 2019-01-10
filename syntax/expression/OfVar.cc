#include <ymir/syntax/expression/OfVar.hh>

namespace syntax {

    OfVar::OfVar () : _right (Expression::empty ()) {}

    Expression OfVar::init (const OfVar & alloc) {
	auto ret = new (Z0) OfVar ();
	ret-> _token = alloc._token;
	ret-> _right = alloc._right;
	return Expression {ret};
    }
    
    Expression OfVar::init (const lexing::Word & location, const Expression & type) {
	auto ret = new (Z0) OfVar ();
	ret-> _token = location;
	ret-> _right = type;
	return Expression {ret};
    }

    Expression OfVar::clone () const {
	return OfVar::init (*this);
    }

    bool OfVar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	OfVar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
