#include <ymir/syntax/expression/AliasVar.hh>

namespace syntax {

    AliasVar::AliasVar () :
	IExpression (lexing::Word::eof ())
    {}
    
    AliasVar::AliasVar (const lexing::Word & loc) :
	IExpression (loc)
    {}
        
    Expression AliasVar::init (const lexing::Word & location) {
	return Expression {new (NO_GC) AliasVar (location)};
    }

    void AliasVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<AliasVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
    }

    std::string AliasVar::prettyString () const {
	return "alias " + this-> getLocation ().getStr ();
    }

    const std::set <std::string> & AliasVar::computeSubVarNames () {
	this-> setSubVarNames ({this-> getLocation ().getStr ()});
	return this-> getSubVarNames ();
    }

}
