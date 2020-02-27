#include <ymir/syntax/expression/While.hh>

namespace syntax {

    While::While () :
	IExpression (lexing::Word::eof ()),
	_test (Expression::empty ()),
	_content (Expression::empty ())
    {}    
    
    While::While (const lexing::Word & loc, const Expression & test, const Expression & content, bool isDo) :
	IExpression (loc),
	_test (test),
	_content (content),
	_isDo (isDo)
    {}

    Expression While::init (const lexing::Word & location, const Expression & test, const Expression & content, bool isDo) {
	return Expression {new (Z0) While (location, test, content, isDo)};
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

    const Expression & While::getTest () const {
	return this-> _test;
    }

    const Expression & While::getContent () const {
	return this-> _content;
    }

    bool While::isDo () const {
	return this-> _isDo;
    }

}
