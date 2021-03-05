#include <ymir/syntax/expression/Var.hh>

namespace syntax {

    Var::Var () :
	IExpression (lexing::Word::eof ())
    {}
    
    Var::Var (const lexing::Word & loc) :
	IExpression (loc)
    {}
    
    Expression Var::init (const lexing::Word & location) {
	return Expression {new (NO_GC) Var (location)};
    }
    
    void Var::treePrint (Ymir::OutBuffer & stream, int i) const {	
	stream.writef ("%*<Var> : ", i, '\t');
	stream.writeln (this-> getLocation ());
    }

    const lexing::Word & Var::getName () const {
	return this-> getLocation ();
    }

    std::string Var::prettyString () const {
	return this-> getLocation ().getStr ();
    }

    const std::set <std::string> & Var::computeSubVarNames () {
	this-> setSubVarNames ({this-> getLocation ().getStr ()});
	return this-> getSubVarNames ();
    }
    
}
