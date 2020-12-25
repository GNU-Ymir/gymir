#include <ymir/syntax/declaration/MacroRule.hh>

namespace syntax {

    MacroRule::MacroRule () :
	IDeclaration (lexing::Word::eof (), ""),
	_rule (Expression::empty ())
    {}

    MacroRule::MacroRule (const lexing::Word & loc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector<Expression> & skips) :
	IDeclaration (loc, comment),
	_rule (rule),
	_content (content),
	_skips (skips)
    {}

    Declaration MacroRule::init (const lexing::Word & loc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector <Expression> & skips) {
	return Declaration {new (NO_GC) MacroRule (loc, comment, rule, content, skips)};
    }

    Declaration MacroRule::init (const syntax::MacroRule & rule) {
	return Declaration {new (NO_GC) MacroRule (rule)};
    }

    bool MacroRule::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MacroRule thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
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

    const Expression & MacroRule::getRule () const {
	return this-> _rule;
    }

    const std::vector<Expression> & MacroRule::getSkips () const {
	return this-> _skips;
    }
    
}
