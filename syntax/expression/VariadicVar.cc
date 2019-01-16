#include <ymir/syntax/expression/VariadicVar.hh>

namespace syntax {

    VariadicVar::VariadicVar () {}

    Expression VariadicVar::init (const VariadicVar & alloc) {
	auto ret = new (Z0) VariadicVar ();
	ret-> _token = alloc._token;
	ret-> _isValue = alloc._isValue;
	return Expression {ret};
    }
    
    Expression VariadicVar::init (const lexing::Word & location, bool isValue) {
	auto ret = new (Z0) VariadicVar ();
	ret-> _token = location;
	ret-> _isValue = isValue;
	return Expression {ret};
    }

    Expression VariadicVar::clone () const {
	return VariadicVar::init (*this);
    }

    bool VariadicVar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	VariadicVar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }


    void VariadicVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<VariadicVar> ", i, '\t');
	stream.writeln (this-> _token, ' ', this-> _isValue);
    }
}
