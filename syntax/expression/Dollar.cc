#include <ymir/syntax/expression/Dollar.hh>

namespace syntax {

    Dollar::Dollar () :
	IExpression (lexing::Word::eof ())
    {}
    
    Dollar::Dollar (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Dollar::init (const lexing::Word & location) {
	return Expression {new (Z0) Dollar (location)};
    }

    Expression Dollar::clone () const {
	return Expression {new (Z0) Dollar (*this)};
    }

    bool Dollar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Dollar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Dollar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Dollar>", i, '\t');
    }
    
}
