#include <ymir/syntax/expression/Fixed.hh>

namespace syntax {

    Fixed::Fixed () {}

    Expression Fixed::init (const Fixed & alloc) {
	auto ret = new (Z0) Fixed ();
	ret-> _token = alloc._token;
	return Expression {ret};
    }

    Expression Fixed::init (const lexing::Word & location) {
	auto ret = new (Z0) Fixed ();
	ret-> _token = location;
	return Expression {ret};
    }

    Expression Fixed::clone () const {
	return Fixed::init (*this);
    }

    bool Fixed::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Fixed thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
