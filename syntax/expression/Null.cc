#include <ymir/syntax/expression/Null.hh>

namespace syntax {

    Null::Null () {}

    Expression Null::init (const Null & alloc) {
	auto ret = new (Z0) Null ();
	ret-> _token = alloc._token;
	return Expression {ret};
    }

    Expression Null::init (const lexing::Word & location) {
	auto ret = new (Z0) Null ();
	ret-> _token = location;
	return Expression {ret};
    }

    Expression Null::clone () const {
	return Null::init (*this);
    }

    bool Null::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Null thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Null::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Null>", i, '\t');
    }
    
}
