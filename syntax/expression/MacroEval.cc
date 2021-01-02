#include <ymir/syntax/expression/MacroEval.hh>

namespace syntax {

    MacroEval::MacroEval () :
	IExpression (lexing::Word::eof ()),
	_end (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    MacroEval::MacroEval (const lexing::Word & loc, const lexing::Word & end, const Expression & content) :
	IExpression (loc),
	_end (end),
	_content (content)
    {}
        
    Expression MacroEval::init (const lexing::Word & location, const lexing::Word & end, const Expression & content) {
	return Expression {new (NO_GC) MacroEval (location, end, content)};
    }

    void MacroEval::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<MacroEval> ", i, '\t');
	this-> _content.treePrint (stream, i + 1);
    }

    std::string MacroEval::prettyString () const {
	Ymir::OutBuffer buf;
	buf.write ("#(");
	buf.write (this-> _content.prettyString ());    
	buf.write (")");
	return buf.str ();
    }

    const lexing::Word & MacroEval::getEnd () const {
	return this-> _end;
    }
    
    const Expression & MacroEval::getContent () const {
	return this-> _content;
    }

}
