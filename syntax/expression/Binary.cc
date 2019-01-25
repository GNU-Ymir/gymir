#include <ymir/syntax/expression/Binary.hh>

namespace syntax {

    Binary::Binary () :
	IExpression (lexing::Word::eof ()),
	_left (Expression::empty ()),
	_right (Expression::empty ()),
	_type (Expression::empty ())
    {}
    
    Binary::Binary (const lexing::Word & loc) :
	IExpression (loc),
	_left (Expression::empty ()),
	_right (Expression::empty ()),
	_type (Expression::empty ())
    {}

    Expression Binary::init (const lexing::Word & location, const Expression & left, const Expression & right, const Expression & type) {
	auto ret = new (Z0) Binary (location);
	ret-> _op = location;
	ret-> _left = left;
	ret-> _right = right;
	ret-> _type = type;
	return Expression {ret};
    }

    Expression Binary::clone () const {
	return Expression {new (Z0) Binary (*this)};
    }

    bool Binary::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Binary thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Binary::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Binary> ", i, '\t');
	stream.writeln (this-> _op);
	stream.writefln ("%*<Type> ", i + 1, '\t');
	this-> _type.treePrint (stream, i + 2);
	this-> _left.treePrint (stream, i + 1);
	this-> _right.treePrint (stream, i + 1);
    }
    
}
