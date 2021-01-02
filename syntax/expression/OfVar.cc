#include <ymir/syntax/expression/OfVar.hh>

namespace syntax {

    OfVar::OfVar () :
	IExpression (lexing::Word::eof ()),
	_right (Expression::empty ())
    {}
    
    OfVar::OfVar (const lexing::Word & loc, const Expression & type) :
	IExpression (loc),
	_right (type)
    {}
        
    Expression OfVar::init (const lexing::Word & location, const Expression & type) {
	return Expression {new (NO_GC) OfVar (location, type)};
    }

    void OfVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<OfVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _right.treePrint (stream, i + 1);
    }

    const Expression & OfVar::getType () const {
	return this-> _right;
    }

    std::string OfVar::prettyString () const {
	return this-> getLocation ().getStr () + " of " + this-> _right.prettyString ();
    }
}
