#include <ymir/syntax/expression/MacroCall.hh>

namespace syntax {

    MacroCall::MacroCall () :
	IExpression (lexing::Word::eof ()),
	_end (lexing::Word::eof ()),
	_left (Expression::empty ()),
	_content ("") {}

    MacroCall::MacroCall (const lexing::Word & loc, const lexing::Word & end, const Expression & left, const std::string & content) :
	IExpression (loc),
	_end (end),
	_left (left),
	_content (content)
    {}

    Expression MacroCall::init (const lexing::Word & loc, const lexing::Word & end, const Expression & left, const std::string & content) {
	return Expression {new (NO_GC) MacroCall (loc, end, left, content)};
    }
    
    const Expression & MacroCall::getLeft () const {
	return this-> _left;
    }

    const lexing::Word & MacroCall::getEnd () const {
	return this-> _end;
    }
    
    void MacroCall::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<MacroCall> ", i, '\t');
	this-> _left.treePrint (stream, i + 1);
	stream.writefln ("%*<Content> : %", i+1, '\t', this-> _content);
    }

    std::string MacroCall::prettyString () const {
	return Ymir::format ("% (%)", this-> _left.prettyString (), this-> _content);
    }

    const std::string & MacroCall::getContent () const {
	return this-> _content;
    }
    
    const std::set <std::string> & MacroCall::computeSubVarNames () {
	this-> setSubVarNames (this-> _left.getSubVarNames ());
	return this-> getSubVarNames ();
    }
}
