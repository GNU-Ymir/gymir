#include <ymir/syntax/expression/MacroResult.hh>

namespace syntax {

    MacroResult::MacroResult () :
	IExpression (lexing::Word::eof ())
    {}
    
    MacroResult::MacroResult (const lexing::Word & loc, const std::string & content) :
	IExpression (loc),
	_content (content)
    {}
        
    Expression MacroResult::init (const lexing::Word & location, const std::string & content) {
	return Expression {new (NO_GC) MacroResult (location, content)};
    }

    void MacroResult::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<MacroResult> ", i, '\t');
	stream.writefln ("%*%", i+1, '\t', this-> getLocation ());
	stream.writefln ("%*%", i+1, '\t', this-> _content);
    }

    std::string MacroResult::prettyString () const {
	return this-> _content;
    }

    const std::string & MacroResult::getContent () const {
	return this-> _content;
    }
	
    const std::set <std::string> & MacroResult::computeSubVarNames () {
	this-> setSubVarNames ({});
	return this-> getSubVarNames ();
    }
}
