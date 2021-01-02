#include <ymir/syntax/expression/StructVar.hh>

namespace syntax {

    StructVar::StructVar () :
	IExpression (lexing::Word::eof ())
    {}
    
    StructVar::StructVar (const lexing::Word & loc) :
	IExpression (loc)
    {}
        
    Expression StructVar::init (const lexing::Word & location) {
	auto ret = new (NO_GC) StructVar (location);
	return Expression {ret};
    }

    void StructVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<StructVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
    }

    std::string StructVar::prettyString () const {
	return "struct " + this-> getLocation ().getStr ();
    }
}
