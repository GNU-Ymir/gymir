#include <ymir/syntax/expression/Char.hh>

namespace syntax {

    Char::Char () :
	IExpression (lexing::Word::eof ())
    {}
    
    Char::Char (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Char::init (const lexing::Word & location, const std::string & sequence) {
	auto ret = new (Z0) Char (location);	
	ret-> _sequence = sequence;
	return Expression {ret};
    }

    Expression Char::clone () const {
	return Expression {new (Z0) Char (*this)};
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
    
}
