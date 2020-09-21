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
	return Expression {new (Z0) ImplVar (location, type)};
    }

    bool ImplVar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	ImplVar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
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
	return this-> getLocation ().str + " impl " + this-> _right.prettyString ();
    }
}
