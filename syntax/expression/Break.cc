#include <ymir/syntax/expression/Break.hh>

namespace syntax {

    Break::Break () :
	IExpression (lexing::Word::eof ()),
	_value (Expression::empty ())
    {}
    
    Break::Break (const lexing::Word & loc, const Expression & value) :
	IExpression (loc),
	_value (value)
    {}

    Expression Break::init (const lexing::Word & location, const Expression & value) {
	return Expression {new (NO_GC) Break (location, value)};	
    }

    void Break::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Break>", i, '\t');
	this-> _value.treePrint (stream, i + 1);
    }

    const Expression & Break::getValue () const {
	return this-> _value;
    }

    std::string Break::prettyString () const {
	if (this-> _value.isEmpty ())
	    return "break";
	else 
	return Ymir::format ("break %", this-> _value.prettyString ());
    }
    
}
