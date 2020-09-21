#include <ymir/syntax/expression/MacroVar.hh>

namespace syntax {

    MacroVar::MacroVar () :
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    MacroVar::MacroVar (const lexing::Word & loc, const Expression & content) :
	IExpression (loc),
	_content (content)
    {}
        
    Expression MacroVar::init (const lexing::Word & location, const Expression & content) {
	return Expression {new (Z0) MacroVar (location, content)};
    }

    bool MacroVar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MacroVar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void MacroVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<MacroVar> ", i, '\t');
	stream.writefln ("%*%", i+1, '\t', this-> getLocation ());
	this-> _content.treePrint (stream, i + 1);
    }

    std::string MacroVar::prettyString () const {
	return this-> getLocation ().str + "=" + this-> _content.prettyString ();
    }

    const Expression & MacroVar::getContent () const {
	return this-> _content;
    }
    
}
