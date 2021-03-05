#include <ymir/syntax/declaration/MacroRule.hh>

namespace syntax {

    MacroRule::MacroRule (const lexing::Word & loc, const lexing::Word & contentLoc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector<Expression> & skips) :
	IDeclaration (loc, comment),
	_rule (rule),
	_contentLoc (contentLoc),
	_content (content),
	_skips (skips)
    {}

    Declaration MacroRule::init (const lexing::Word & loc, const lexing::Word & contentLoc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector <Expression> & skips) {
	return Declaration {new (NO_GC) MacroRule (loc, contentLoc, comment, rule, content, skips)};
    }

    Declaration MacroRule::init (const syntax::MacroRule & rule) {
	return Declaration {new (NO_GC) MacroRule (rule)};
    }

    void MacroRule::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<MacroRule> : ", this-> getLocation ());
	this-> _rule.treePrint (stream, i+1);
	stream.writefln ("%*%", i, '\t', this-> _content);
    }

    const std::string & MacroRule::getContent () const {
	return this-> _content;
    }

    const lexing::Word & MacroRule::getContentLoc () const {
	return this-> _contentLoc;
    }
    
    const Expression & MacroRule::getRule () const {
	return this-> _rule;
    }

    const std::vector<Expression> & MacroRule::getSkips () const {
	return this-> _skips;
    }
    
}
