#include <ymir/syntax/expression/String.hh>

namespace syntax {
    
    String::String (const lexing::Word & loc, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix) :
	IExpression (loc),
	_end (end),
	_sequence (sequence),
	_suffix (suffix)
    {}

    Expression String::init (const lexing::Word & location, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix) {
	return Expression {new (NO_GC) String (location, end, sequence, suffix)};	
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
	return this-> _sequence.getStr ();
    }
    
}
