#include <ymir/syntax/expression/String.hh>

namespace syntax {

    String::String () :
	IExpression (lexing::Word::eof ())
    {}
    
    String::String (const lexing::Word & loc, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix) :
	IExpression (loc),
	_end (end),
	_sequence (sequence),
	_suffix (suffix)
    {}

    Expression String::init (const lexing::Word & location, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix) {
	return Expression {new String (location, end, sequence, suffix)};	
    }

    Expression String::clone () const {
	return Expression {new String (*this)};
    }

    bool String::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	String thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
    void String::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<String> ", i, '\t');
	stream.writeln (this-> getLocation (), " ", this-> _sequence);
    }

    const lexing::Word & String::getSuffix () const {
	return this-> _suffix;
    }
    
    const lexing::Word & String::getSequence () const {
	return this-> _sequence;
    }

    std::string String::prettyString () const {
	return this-> _sequence.str;
    }
    
}
