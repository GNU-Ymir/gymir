#include <ymir/syntax/expression/Cast.hh>

namespace syntax {

    Cast::Cast () :
	_type (Expression::empty ()),
	_content (Expression::empty ())
    {}

    Expression Cast::init (const Cast & alloc) {
	auto ret = new (Z0) Cast ();
	ret-> _op = alloc._op;
	ret-> _type = alloc._type;
	ret-> _content = alloc._content;
	return Expression {ret};
    }

    Expression Cast::init (const lexing::Word & location, const Expression & type, const Expression & content) {
	auto ret = new (Z0) Cast ();
	ret-> _op = location;
	ret-> _type = type;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression Cast::clone () const {
	return Cast::init (*this);
    }

    bool Cast::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Cast thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
