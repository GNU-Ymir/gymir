#include <ymir/syntax/expression/Binary.hh>

namespace syntax {

    Binary::Binary () :
	_left (Expression::empty ()),
	_right (Expression::empty ()),
	_type (Expression::empty ())
    {}

    Expression Binary::init (const Binary & alloc) {
	auto ret = new (Z0) Binary ();
	ret-> _op = alloc._op;
	ret-> _left = alloc._left;
	ret-> _right = alloc._right;
	ret-> _type = alloc._type;
	return Expression {ret};
    }

    Expression Binary::init (const lexing::Word & location, const Expression & left, const Expression & right, const Expression & type) {
	auto ret = new (Z0) Binary ();
	ret-> _op = location;
	ret-> _left = left;
	ret-> _right = right;
	ret-> _type = type;
	return Expression {ret};
    }

    Expression Binary::clone () const {
	return Binary::init (*this);
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
