#include <ymir/syntax/expression/Throw.hh>

namespace syntax {

    Throw::Throw () :
	IExpression (lexing::Word::eof ()),
	_value (Expression::empty ())
    {}
    
    Throw::Throw (const lexing::Word & loc, const Expression & value) :
	IExpression (loc),
	_value (value)
    {}

    Expression Throw::init (const lexing::Word & location, const Expression & value) {
	return Expression {new (NO_GC) Throw (location, value)};
    }

    void Throw::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Throw> ", i, '\t');
	this-> _value.treePrint (stream, i + 1);
    }

    const Expression & Throw::getValue () const {
	return this-> _value;
    }

    std::string Throw::prettyString () const {
	return Ymir::format ("throw %", this-> _value.prettyString ());
    }
    
}
