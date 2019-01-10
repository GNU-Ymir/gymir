#include <ymir/syntax/expression/MacroCall.hh>

namespace syntax {

    MacroCall::MacroCall () : _left (Expression::empty ()) {}

    Expression MacroCall::init (const MacroCall & alloc) {
	auto ret = new (Z0) MacroCall ();
	ret-> _location = alloc._location;
	ret-> _left = alloc._left;
	ret-> _content = alloc._content;
	return Expression {ret};
    }

    Expression MacroCall::init (const lexing::Word & location, const Expression & left, const std::vector <lexing::Word> & content) {
	auto ret = new (Z0) MacroCall ();
	ret-> _location = location;
	ret-> _left = left;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression MacroCall::clone () const {
	return MacroCall::init (*this);
    }

    bool MacroCall::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MacroCall thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
