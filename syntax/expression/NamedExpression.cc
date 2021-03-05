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
	return Expression {new (NO_GC) NamedExpression (location, content)};
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
	return Ymir::format ("% -> %", this-> getLocation ().getStr (), this-> _content.prettyString ());
    }
    
}
