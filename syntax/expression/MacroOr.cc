#include <ymir/syntax/expression/MacroOr.hh>

namespace syntax {

    MacroOr::MacroOr () :
	IExpression (lexing::Word::eof ()),
	_left (Expression::empty ()),
	_right (Expression::empty ())
    {}
    
    MacroOr::MacroOr (const lexing::Word & loc, const Expression & left, const Expression & right) :
	IExpression (loc),
	_left (left),
	_right (right)
    {}
        
    Expression MacroOr::init (const lexing::Word & location, const Expression & left, const Expression & right) {
	return Expression {new (Z0) MacroOr (location, left, right)};
    }

    bool MacroOr::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MacroOr thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void MacroOr::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<MacroOr> ", i, '\t');
	this-> _left.treePrint (stream, i+1);
	this-> _right.treePrint (stream, i+1);
    }

    const Expression & MacroOr::getLeft () const {
	return this-> _left;
    }

    const Expression & MacroOr::getRight () const {
	return this-> _right;
    }
    
    std::string MacroOr::prettyString () const {
	return this-> _left.prettyString () + " | " + this-> _right.prettyString ();
    }
}
