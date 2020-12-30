#include <ymir/syntax/expression/Unary.hh>

namespace syntax {

    Unary::Unary () : 
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    Unary::Unary (const lexing::Word & loc, const Expression & content) :
	IExpression (loc),
	_content (content)
    {}

    Expression Unary::init (const lexing::Word & location, const Expression & content) {
	return Expression {new (NO_GC) Unary (location, content)};
    }

    bool Unary::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Unary thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Unary::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Unary>", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _content.treePrint (stream, i + 1);
    }

    const lexing::Word & Unary::getOperator () const {
	return this-> getLocation ();
    }
    
    const Expression & Unary::getContent () const {
	return this-> _content;
    }

    std::string Unary::prettyString () const {
	return Ymir::format ("% (%)", this-> getLocation ().getStr (), this-> _content.prettyString ());
    }
    
}
