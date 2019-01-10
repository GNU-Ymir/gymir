#include <ymir/syntax/expression/Match.hh>

namespace syntax {

    Match::Match () {}

    Expression Match::init (const Match & alloc) {
	auto ret = new (Z0) Match ();
	ret-> _token = alloc._token;
	return Expression {ret};
    }

    Expression Match::init (const lexing::Word & location) {
	auto ret = new (Z0) Match ();
	ret-> _token = location;
	return Expression {ret};
    }

    Expression Match::clone () const {
	return Match::init (*this);
    }

    bool Match::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Match thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
