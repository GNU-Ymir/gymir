#include <ymir/syntax/expression/Null.hh>

namespace syntax {

    Null::Null () :
	IExpression (lexing::Word::eof ())
    {}
    
    Null::Null (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Null::init (const lexing::Word & location) {
	return Expression {new (NO_GC) Null (location)};
    }

    void Null::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Null>", i, '\t');
    }

    std::string Null::prettyString () const {
	return "null";
    }
    
}
