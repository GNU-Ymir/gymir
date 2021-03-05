#include <ymir/syntax/expression/MacroVar.hh>

namespace syntax {

    MacroVar::MacroVar () :
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    MacroVar::MacroVar (const lexing::Word & loc, const Expression & content) :
	IExpression (loc),
	_content (content)
    {}
        
    Expression MacroVar::init (const lexing::Word & location, const Expression & content) {
	return Expression {new (NO_GC) MacroVar (location, content)};
    }

    void MacroVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<MacroVar> ", i, '\t');
	stream.writefln ("%*%", i+1, '\t', this-> getLocation ());
	this-> _content.treePrint (stream, i + 1);
    }

    std::string MacroVar::prettyString () const {
	return this-> getLocation ().getStr () + "=" + this-> _content.prettyString ();
    }

    const Expression & MacroVar::getContent () const {
	return this-> _content;
    }

    const std::set <std::string> & MacroVar::computeSubVarNames () {
	this-> setSubVarNames (this-> _content.getSubVarNames ());
	return this-> getSubVarNames ();
    }
}
