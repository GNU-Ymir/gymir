#include <ymir/syntax/expression/OfVar.hh>

namespace syntax {

    OfVar::OfVar () :
	IExpression (lexing::Word::eof ()),
	_right (Expression::empty ()),
	_isOver (false)
    {}
    
    OfVar::OfVar (const lexing::Word & loc, const Expression & type, bool isOver) :
	IExpression (loc),
	_right (type),
	_isOver (isOver)
    {}
        
    Expression OfVar::init (const lexing::Word & location, const Expression & type, bool isOver) {
	return Expression {new (NO_GC) OfVar (location, type, isOver)};
    }

    void OfVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<OfVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _right.treePrint (stream, i + 1);
    }

    bool OfVar::isOver () const {
	return this-> _isOver;
    }
    
    const Expression & OfVar::getType () const {
	return this-> _right;
    }

    std::string OfVar::prettyString () const {
	if (this-> _isOver)
	    return this-> getLocation ().getStr () + " over " + this-> _right.prettyString ();
	else
	    return this-> getLocation ().getStr () + " of " + this-> _right.prettyString ();
    }
}
