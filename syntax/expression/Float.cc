#include <ymir/syntax/expression/Float.hh>

namespace syntax {

    Float::Float () :
	IExpression (lexing::Word::eof ())
    {}
    
    Float::Float (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Float::init (const lexing::Word & location, const lexing::Word & decPart, const lexing::Word & suffix) {
	auto ret = new (Z0) Float (location);
	ret-> _decPart = decPart;
	ret-> _suffix = suffix;
	return Expression {ret};
    }

    Expression Float::clone () const {
	return Expression {new Float (*this)};
    }

    bool Float::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Float thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Float::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Float> ", i, '\t');
	stream.writeln (this-> getLocation (), " ", this-> _decPart, " ", this-> _suffix);
    }
    
}
