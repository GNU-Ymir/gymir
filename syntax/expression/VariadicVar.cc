#include <ymir/syntax/expression/VariadicVar.hh>

namespace syntax {

    VariadicVar::VariadicVar () :
	IExpression (lexing::Word::eof ())
    {}
    
    VariadicVar::VariadicVar (const lexing::Word & loc, bool isValue) :
	IExpression (loc),
	_isValue (isValue)
    {}
    
    Expression VariadicVar::init (const lexing::Word & location, bool isValue) {
	return Expression {new (NO_GC) VariadicVar (location, isValue)};
    }

    void VariadicVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<VariadicVar> ", i, '\t');
	stream.writeln (this-> getLocation (), ' ', this-> _isValue);
    }

    std::string VariadicVar::prettyString () const {
	if (this-> _isValue)
	    return Ymir::format ("%:...", this-> getLocation ().getStr ());
	else
	    return Ymir::format ("%...", this-> getLocation ().getStr ());
    }
	
    const std::set <std::string> & VariadicVar::computeSubVarNames () {
	this-> setSubVarNames ({this-> getLocation ().getStr ()});
	return this-> getSubVarNames ();
    }
    
}
