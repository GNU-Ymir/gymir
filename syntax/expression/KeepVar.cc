#include <ymir/syntax/expression/KeepVar.hh>

namespace syntax {

    KeepVar::KeepVar () :
	IExpression (lexing::Word::eof ())
    {}
    
    KeepVar::KeepVar (const lexing::Word & loc) :
	IExpression (loc)
    {}
        
    Expression KeepVar::init (const lexing::Word & location) {
	return Expression {new (NO_GC) KeepVar (location)};
    }

    void KeepVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<KeepVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
    }

    std::string KeepVar::prettyString () const {
	return "keep " + this-> getLocation ().getStr ();
    }
}
