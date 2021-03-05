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
	return Expression {new (NO_GC) MacroOr (location, left, right)};
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

    const std::set <std::string> & MacroOr::computeSubVarNames () {
	auto lSet = this-> _left.getSubVarNames ();
	auto & rSet = this-> _right.getSubVarNames ();
	lSet.insert (rSet.begin (), rSet.end ());
	this-> setSubVarNames (lSet);
	return this-> getSubVarNames ();
    }
}
