#include <ymir/syntax/expression/Return.hh>

namespace syntax {

    Return::Return () :
	IExpression (lexing::Word::eof ()),
	_value (Expression::empty ())
    {}
    
    Return::Return (const lexing::Word & loc, const Expression & value) :
	IExpression (loc),
	_value (value)
    {}

    Expression Return::init (const lexing::Word & location, const Expression & value) {
	return Expression {new (Z0) Return (location, value)};
    }

    bool Return::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Return thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Return::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Return> ", i, '\t');
	this-> _value.treePrint (stream, i + 1);
    }

    const Expression & Return::getValue () const {
	return this-> _value;
    }
    
}
