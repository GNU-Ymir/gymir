#include <ymir/syntax/expression/Char.hh>

namespace syntax {
    
    Char::Char (const lexing::Word & loc, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix) :
	IExpression (loc),
	_end (end),
	_sequence (sequence),
	_suffix (suffix)
    {}

    Expression Char::init (const lexing::Word & location, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix) {
	return Expression {new (NO_GC) Char (location, end, sequence, suffix)};	
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

    std::string Char::prettyString () const {
	if (this-> _suffix.isEof ())
	    return Ymir::format ("'%'%", this->_sequence.getStr (), this-> _suffix.getStr ());
	else
	    return Ymir::format ("'%'", this->_sequence.getStr ());
    }
    
}
