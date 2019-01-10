#include <ymir/syntax/expression/Lambda.hh>

namespace syntax {

    Lambda::Lambda () :
	_proto (Function::Prototype::init ()),
	_content (Expression::empty ())
    {}

    Expression Lambda::init (const Lambda & alloc) {
	auto ret = new (Z0) Lambda ();
	ret-> _op = alloc._op;
	ret-> _proto = alloc._proto;
	ret-> _content = alloc._content;
	return Expression {ret};
    }

    Expression Lambda::init (const lexing::Word & location, const Function::Prototype & proto, const Expression & content) {
	auto ret = new (Z0) Lambda ();
	ret-> _op = location;
	ret-> _proto = proto;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression Lambda::clone () const {
	return Lambda::init (*this);
    }

    bool Lambda::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Lambda thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
