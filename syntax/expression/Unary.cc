#include <ymir/syntax/expression/Unary.hh>

namespace syntax {

    Unary::Unary () :	
	_content (Expression::empty ())
    {}

    Expression Unary::init (const Unary & alloc) {
	auto ret = new (Z0) Unary ();
	ret-> _op = alloc._op;
	ret-> _content = alloc._content;
	return Expression {ret};
    }

    Expression Unary::init (const lexing::Word & location, const Expression & content) {
	auto ret = new (Z0) Unary ();
	ret-> _op = location;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression Unary::clone () const {
	return Unary::init (*this);
    }

    bool Unary::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Unary thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
