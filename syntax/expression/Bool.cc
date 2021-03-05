#include <ymir/syntax/expression/Bool.hh>

namespace syntax {

    Bool::Bool () :
	IExpression (lexing::Word::eof ())
    {}
    
    Bool::Bool (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Bool::init (const lexing::Word & location) {
	return Expression {new (NO_GC) Bool (location)};
    }

    void Bool::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Bool> ", i, '\t');
	stream.writeln (this-> getLocation ());
    }
    

    std::string Bool::prettyString () const {
	return this-> getLocation ().getStr ();
    }
}
