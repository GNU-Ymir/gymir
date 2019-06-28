#include <ymir/syntax/expression/Assert.hh>

namespace syntax {

    Assert::Assert () :
	IExpression (lexing::Word::eof ()),
	_test (Expression::empty ()),
	_msg (Expression::empty ())
    {}
    
    Assert::Assert (const lexing::Word & loc) :
	IExpression (loc),
	_test (Expression::empty ()),
	_msg (Expression::empty ())
    {}

    Expression Assert::init (const lexing::Word & loc, const Expression & test, const Expression & msg) {
	auto ret = new (Z0) Assert (loc);
	ret-> _test = test;
	ret-> _msg = msg;
	return Expression {ret};
    }

    Expression Assert::clone () const {
	return Expression {new (Z0) Assert (*this)};
    }

    bool Assert::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Assert thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }	    

    const Expression & Assert::getTest () const {
	return this-> _test;
    }

    const Expression & Assert::getMsg () const {
	return this-> _msg;
    }
    
    void Assert::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Assert> ", i, '\t');
	this-> _test.treePrint (stream, i + 1);
	this-> _msg.treePrint (stream, i + 1);
    }
    
}
