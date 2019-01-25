#include <ymir/syntax/expression/Throw.hh>

namespace syntax {

    Throw::Throw () :
	IExpression (lexing::Word::eof ()),
	_value (Expression::empty ())
    {}
    
    Throw::Throw (const lexing::Word & loc) :
	IExpression (loc),
	_value (Expression::empty ())
    {}

    Expression Throw::init (const lexing::Word & location, const Expression & value) {
	auto ret = new (Z0) Throw (location);
	ret-> _value = value;
	return Expression {ret};
    }

    Expression Throw::clone () const {
	return Expression {new Throw (*this)};
    }

    bool Throw::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Throw thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Throw::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Throw> ", i, '\t');
	this-> _value.treePrint (stream, i + 1);
    }
    
}
