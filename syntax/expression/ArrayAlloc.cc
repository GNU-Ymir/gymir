#include <ymir/syntax/expression/ArrayAlloc.hh>

namespace syntax {

    ArrayAlloc::ArrayAlloc () :
	_left (Expression::empty ()),
	_size (Expression::empty ())
    {}

    Expression ArrayAlloc::init (const ArrayAlloc & alloc) {
	auto ret = new (Z0) ArrayAlloc ();
	ret-> _location = alloc._location;
	ret-> _left = alloc._left;
	ret-> _size = alloc._size;
	ret-> _isDynamic = alloc._isDynamic;
	return Expression {ret};
    }

    Expression ArrayAlloc::init (const lexing::Word & location, const Expression & left, const Expression &size, bool isDynamic) {
	auto ret = new (Z0) ArrayAlloc ();
	ret-> _location = location;
	ret-> _left = left;
	ret-> _size = size;
	ret-> _isDynamic = isDynamic;
	return Expression {ret};
    }

    Expression ArrayAlloc::clone () const {
	return ArrayAlloc::init (*this);
    }

    bool ArrayAlloc::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	ArrayAlloc thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void ArrayAlloc::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<ArrayAlloc> ", i, '\t');
	stream.writeln (this-> _isDynamic ? "true" : "false");
	stream.writefln ("%*<Type> ", i + 1, '\t');
	this-> _left.treePrint (stream, i + 2);

	stream.writefln ("%*<Size> ", i + 1, '\t');
	this-> _size.treePrint (stream, i + 2);
    }
    
}
