#include <ymir/syntax/expression/If.hh>

namespace syntax {
        
    If::If () :
	IExpression (lexing::Word::eof ()),
	_test (Expression::empty ()),
	_content (Expression::empty ()),
	_else (Expression::empty ())
    {}

    If::If (const lexing::Word & loc) :
	IExpression (loc),
	_test (Expression::empty ()),
	_content (Expression::empty ()),
	_else (Expression::empty ())
    {}
    
   
    Expression If::init (const lexing::Word & location, const Expression & test, const Expression & content, const Expression & elsePart) {
	auto ret = new (Z0) If (location);
	ret-> _test = test;
	ret-> _content = content;
	ret-> _else = elsePart;
	return Expression {ret};
    }

    Expression If::clone () const {
	return Expression {new If (*this)};
    }

    bool If::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	If thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void If::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<If> ", i, '\t');
	this-> _test.treePrint (stream, i + 1);
	this-> _content.treePrint (stream, i + 1);
	this-> _else.treePrint (stream, i + 1);
    }

    const Expression & If::getTest () const {
	return this-> _test;
    }

    const Expression & If::getContent () const {
	return this-> _content;
    }

    const Expression & If::getElsePart () const {
	return this-> _else;
    }
    
}
