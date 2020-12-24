#include <ymir/syntax/declaration/MacroConstructor.hh>

namespace syntax {

    MacroConstructor::MacroConstructor () :
	IDeclaration (lexing::Word::eof ()),
	_rule (Expression::empty ()),
	_type (Expression::empty ())
    {}

    MacroConstructor::MacroConstructor (const lexing::Word & loc, const Expression & type, const Expression & rule, const std::string & content) :
	IDeclaration (loc),
	_rule (rule),
	_type (type),
	_content (content)
    {}

    Declaration MacroConstructor::init (const lexing::Word & loc, const Expression & type, const Expression & rule, const std::string & content) {
	return Declaration {new (NO_GC) MacroConstructor (loc, type, rule, content)};
    }

    Declaration MacroConstructor::init (const MacroConstructor & other) {
	return init (other.getLocation (), other._type, other._rule, other._content);
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
	this-> _type.treePrint (stream, i+1);
	stream.write (this-> _content);
    }

    const Expression & MacroConstructor::getRule () const {
	return this-> _rule;
    }

    const Expression & MacroConstructor::getType () const {
	return this-> _type;
    }

    const std::string & MacroConstructor::getContent () const {
	return this-> _content;
    }
    
}
