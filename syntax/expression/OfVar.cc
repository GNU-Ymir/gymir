#include <ymir/syntax/expression/OfVar.hh>

namespace syntax {

    OfVar::OfVar () :
	IExpression (lexing::Word::eof ()),
	_right (Expression::empty ())
    {}
    
    OfVar::OfVar (const lexing::Word & loc) :
	IExpression (loc),
	_right (Expression::empty ())
    {}
        
    Expression OfVar::init (const lexing::Word & location, const Expression & type) {
	auto ret = new (Z0) OfVar (location);
	ret-> _right = type;
	return Expression {ret};
    }

    Expression OfVar::clone () const {
	return Expression {new OfVar (*this)};
    }

    bool OfVar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	OfVar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void OfVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<OfVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _right.treePrint (stream, i + 1);
    }

    const Expression & OfVar::getType () const {
	return this-> _right;
    }
}
