#include <ymir/syntax/expression/MacroToken.hh>

namespace syntax {

    MacroToken::MacroToken () :
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    MacroToken::MacroToken (const lexing::Word & loc, const Expression & content) :
	IExpression (loc),
	_content (content)
    {}
        
    Expression MacroToken::init (const lexing::Word & location, const Expression & content) {
	return Expression {new (NO_GC) MacroToken (location, content)};
    }

    void MacroToken::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<MacroToken> ", i, '\t');
	stream.writefln ("%*%", i+1, '\t', this-> getLocation ());
	stream.writefln ("%*%", i+1, '\t', this-> _content.prettyString ());
    }

    std::string MacroToken::prettyString () const {
	return Ymir::format ("'%'", this-> _content.prettyString ());
    }

    const Expression & MacroToken::getContent () const {
	return this-> _content;
    }
}
