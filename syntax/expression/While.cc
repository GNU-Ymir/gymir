#include <ymir/syntax/expression/While.hh>

namespace syntax {

    While::While () :
	_test (Expression::empty ()),
	_content (Expression::empty ())
    {}

    Expression While::init (const While & wh) {
	auto ret = new (Z0) While ();
	ret-> _location = wh._location;
	ret-> _test = wh._test;
	ret-> _content = wh._content;
	return Expression {ret};
    }

    Expression While::init (const lexing::Word & location, const Expression & test, const Expression & content) {
	auto ret = new (Z0) While ();
	ret-> _location = location;
	ret-> _test = test;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression While::clone () const {
	return While::init (*this);
    }

    bool While::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	While thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void While::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<While>", i, '\t');
	this-> _test.treePrint (stream, i + 1);
	this-> _content.treePrint (stream, i + 1);
    }
}
