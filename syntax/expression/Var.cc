#include <ymir/syntax/expression/Var.hh>

namespace syntax {

    Var::Var ()  {}

    Expression Var::init (const Var & alloc) {
	auto ret = new (Z0) Var ();
	ret-> _token = alloc._token;
	return Expression {ret};
    }
    
    Expression Var::init (const lexing::Word & location) {
	auto ret = new (Z0) Var ();
	ret-> _token = location;
	return Expression {ret};
    }

    Expression Var::clone () const {
	return Var::init (*this);
    }

    bool Var::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Var thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
