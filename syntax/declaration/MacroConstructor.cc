#include <ymir/syntax/declaration/MacroConstructor.hh>

namespace syntax {

    MacroConstructor::MacroConstructor () :
	IDeclaration (lexing::Word::eof (), ""),
	_rule (Expression::empty ())
    {}

    MacroConstructor::MacroConstructor (const lexing::Word & loc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector <Expression> & skips) :
	IDeclaration (loc, comment),
	_rule (rule),
	_content (content),
	_skips (skips)
    {}

    Declaration MacroConstructor::init (const lexing::Word & loc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector <Expression> & skips) {
	return Declaration {new (NO_GC) MacroConstructor (loc, comment, rule, content, skips)};
    }

    Declaration MacroConstructor::init (const MacroConstructor & other) {
	return Declaration {new (NO_GC) MacroConstructor (other)};
    }

    bool MacroConstructor::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MacroConstructor thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    void MacroConstructor::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<MacroConstructor> : ", this-> getLocation ());
	this-> _rule.treePrint (stream, i+1);
	stream.write (this-> _content);
    }

    const Expression & MacroConstructor::getRule () const {
	return this-> _rule;
    }

    const std::string & MacroConstructor::getContent () const {
	return this-> _content;
    }

    const std::vector <Expression> & MacroConstructor::getSkips () const {
	return this-> _skips;
    }
    
}
