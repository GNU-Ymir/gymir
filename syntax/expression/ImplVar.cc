#include <ymir/syntax/expression/ImplVar.hh>

namespace syntax {

    ImplVar::ImplVar () :
	IExpression (lexing::Word::eof ()),
	_right (Expression::empty ())
    {}
    
    ImplVar::ImplVar (const lexing::Word & loc, const Expression & type) :
	IExpression (loc),
	_right (type)
    {}
        
    Expression ImplVar::init (const lexing::Word & location, const Expression & type) {
	return Expression {new (NO_GC) ImplVar (location, type)};
    }

    void ImplVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<ImplVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _right.treePrint (stream, i + 1);
    }

    const Expression & ImplVar::getType () const {
	return this-> _right;
    }

    std::string ImplVar::prettyString () const {
	return this-> getLocation ().getStr () + " impl " + this-> _right.prettyString ();
    }
}
