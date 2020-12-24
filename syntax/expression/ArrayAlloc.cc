#include <ymir/syntax/expression/ArrayAlloc.hh>

namespace syntax {
   
    ArrayAlloc::ArrayAlloc () :
	IExpression (lexing::Word::eof ()),
	_left (Expression::empty ()),
	_size (Expression::empty ())
    {}

    ArrayAlloc::ArrayAlloc (const lexing::Word & loc, const Expression & left, const Expression & size, bool isDyn) :
	IExpression (loc),
	_left (left),
	_size (size),
	_isDynamic (isDyn)
    {}

    Expression ArrayAlloc::init (const lexing::Word & location, const Expression & left, const Expression &size, bool isDynamic) {
	return Expression {new (NO_GC) ArrayAlloc (location, left, size, isDynamic)};
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

    const Expression & ArrayAlloc::getLeft () const {
	return this-> _left;
    }

    const Expression & ArrayAlloc::getSize () const {
	return this-> _size;
    }

    bool ArrayAlloc::isDynamic () const {
	return this-> _isDynamic;	
    }

    std::string ArrayAlloc::prettyString () const {
	if (!this-> _isDynamic)
	    return Ymir::format ("[%; %]", this-> _left.prettyString (), this-> _size.prettyString ());
	else
	    return Ymir::format ("[%; new %]", this-> _left.prettyString (), this-> _size.prettyString ());

    }
    
}
