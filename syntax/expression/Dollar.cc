#include <ymir/syntax/expression/Dollar.hh>

namespace syntax {

    Dollar::Dollar () :
	IExpression (lexing::Word::eof ())
    {}
    
    Dollar::Dollar (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Dollar::init (const lexing::Word & location) {
	return Expression {new (NO_GC) Dollar (location)};
    }

    void Dollar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Dollar>", i, '\t');
    }

    std::string Dollar::prettyString () const {
	return "$";
    }

    const std::set <std::string> & Dollar::computeSubVarNames () {
	this-> setSubVarNames ({});
	return this-> getSubVarNames ();
    }
    
}
