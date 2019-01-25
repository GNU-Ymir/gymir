#include <ymir/syntax/expression/Match.hh>

namespace syntax {

    Match::Match () :
	IExpression (lexing::Word::eof ())
    {}
    
    Match::Match (const lexing::Word & loc) :
	IExpression (loc)
    {}
    
    Expression Match::init (const lexing::Word & location) {
	return Expression {new (Z0) Match (location)};
    }

    Expression Match::clone () const {
	return Expression {new Match (*this)};
    }

    bool Match::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Match thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
