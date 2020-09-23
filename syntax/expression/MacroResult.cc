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
	return Expression {new (Z0) MacroResult (location, content)};
    }

    bool MacroResult::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MacroResult thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
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
}
