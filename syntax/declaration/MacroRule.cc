#include <ymir/syntax/declaration/MacroRule.hh>

namespace syntax {

    MacroRule::MacroRule () :
	IDeclaration (lexing::Word::eof ()),
	_rule (Expression::empty ()),
	_type (Expression::empty ()),
	_content (Expression::empty ())
    {}

    MacroRule::MacroRule (const lexing::Word & loc, const Expression & rule, const Expression & type, const Expression & content) :
	IDeclaration (loc),
	_rule (rule),
	_type (type),
	_content (content)
    {}

    Declaration MacroRule::init (const lexing::Word & loc, const Expression & rule, const Expression & type, const Expression & content) {
	return Declaration {new (Z0) MacroRule (loc, rule, type, content)};
    }

    Declaration MacroRule::init (const syntax::MacroRule & rule) {
	return Declaration {new (Z0) MacroRule (rule.getLocation (), rule._rule, rule._type, rule._content)};
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
	this-> _type.treePrint (stream, i+1);
	this-> _content.treePrint (stream, i+1);
    }

}
