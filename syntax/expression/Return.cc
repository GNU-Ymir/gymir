#include <ymir/syntax/expression/Return.hh>

namespace syntax {

    Return::Return () :
	IExpression (lexing::Word::eof ()),
	_value (Expression::empty ())
    {}
    
    Return::Return (const lexing::Word & loc, const Expression & value) :
	IExpression (loc),
	_value (value)
    {}

    Expression Return::init (const lexing::Word & location, const Expression & value) {
	return Expression {new (NO_GC) Return (location, value)};
    }

    void Return::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Return> ", i, '\t');
	this-> _value.treePrint (stream, i + 1);
    }

    const Expression & Return::getValue () const {
	return this-> _value;
    }

    std::string Return::prettyString () const {
	if (this-> _value.isEmpty ()) {
	    return "return";
	} else {
	    return Ymir::format ("return %", this-> _value.prettyString ());
	}
    }
    
}
