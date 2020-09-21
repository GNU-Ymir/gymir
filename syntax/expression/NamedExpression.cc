#include <ymir/syntax/expression/NamedExpression.hh>

namespace syntax {

    NamedExpression::NamedExpression () :
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    NamedExpression::NamedExpression (const lexing::Word & loc, const Expression & content) :
	IExpression (loc),
	_content (content)
    {}
    
    Expression NamedExpression::init (const lexing::Word & location, const Expression & content) {
	return Expression {new (Z0) NamedExpression (location, content)};
    }
        
    bool NamedExpression::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	NamedExpression thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void NamedExpression::treePrint (Ymir::OutBuffer & stream, int i) const {	
	stream.writef ("%*<NamedExpression> : ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _content.treePrint (stream, i + 1);
    }

    const Expression & NamedExpression::getContent () const {
	return this-> _content;
    }

    std::string NamedExpression::prettyString () const {
	return Ymir::format ("% -> %", this-> getLocation ().str, this-> _content.prettyString ());
    }
    
}
