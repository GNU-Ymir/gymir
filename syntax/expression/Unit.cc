#include <ymir/syntax/expression/Unit.hh>

namespace syntax {

    Unit::Unit () :
	IExpression (lexing::Word::eof ())
    {}
    
    Unit::Unit (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Unit::init (const lexing::Word & location) {
	return Expression {new (NO_GC) Unit (location)};
    }

    void Unit::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Unit>", i, '\t');
    }

    std::string Unit::prettyString () const {
	return ";";
    }
    
}
