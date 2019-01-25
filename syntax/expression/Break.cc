#include <ymir/syntax/expression/Break.hh>

namespace syntax {

    Break::Break () :
	IExpression (lexing::Word::eof ()),
	_value (Expression::empty ())
    {}
    
    Break::Break (const lexing::Word & loc) :
	IExpression (loc),
	_value (Expression::empty ())
    {}

    Expression Break::init (const lexing::Word & location, const Expression & value) {
	auto br = new (Z0) Break (location);
	br-> _value = value;
	return Expression {br};	
    }
    
    Expression Break::clone () const {
	return Expression {new (Z0) Break (*this)};
    }

    bool Break::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Break thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }	    

    void Break::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Break>", i, '\t');
	this-> _value.treePrint (stream, i + 1);
    }
    
}
