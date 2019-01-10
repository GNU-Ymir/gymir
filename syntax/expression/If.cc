#include <ymir/syntax/expression/If.hh>

namespace syntax {

    If::If () :
	_test (Expression::empty ()),
	_content (Expression::empty ()),
	_else (Expression::empty ())
    {}

    Expression If::init (const If & wh) {
	auto ret = new (Z0) If ();
	ret-> _location = wh._location;
	ret-> _test = wh._test;
	ret-> _content = wh._content;
	ret-> _else = wh._else;
	return Expression {ret};
    }

    Expression If::init (const lexing::Word & location, const Expression & test, const Expression & content, const Expression & elsePart) {
	auto ret = new (Z0) If ();
	ret-> _location = location;
	ret-> _test = test;
	ret-> _content = content;
	ret-> _else = elsePart;
	return Expression {ret};
    }

    Expression If::clone () const {
	return If::init (*this);
    }

    bool If::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	If thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
