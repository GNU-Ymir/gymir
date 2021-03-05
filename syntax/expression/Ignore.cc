#include <ymir/syntax/expression/Ignore.hh>

namespace syntax {

    Ignore::Ignore () :
	IExpression (lexing::Word::eof ())
    {}
    
    Ignore::Ignore (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Ignore::init (const lexing::Word & location) {
	return Expression {new (NO_GC) Ignore (location)};
    }

    void Ignore::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Ignore>", i, '\t');
    }

    std::string Ignore::prettyString () const {
	return "_";
    }
    
}
