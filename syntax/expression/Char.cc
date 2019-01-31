#include <ymir/syntax/expression/Char.hh>

namespace syntax {

    Char::Char () :
	IExpression (lexing::Word::eof ())
    {}
    
    Char::Char (const lexing::Word & loc, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix) :
	IExpression (loc),
	_end (end),
	_sequence (sequence),
	_suffix (suffix)
    {}

    Expression Char::init (const lexing::Word & location, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix) {
	return Expression {new Char (location, end, sequence, suffix)};	
    }

    Expression Char::clone () const {
	return Expression {new Char (*this)};
    }

    bool Char::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Char thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
    void Char::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Char> ", i, '\t');
	stream.writeln (this-> getLocation (), " ", this-> _sequence);
    }

    const lexing::Word & Char::getSuffix () const {
	return this-> _suffix;
    }
    
    const lexing::Word & Char::getSequence () const {
	return this-> _sequence;
    }
    
}
