#include <ymir/syntax/expression/Ignore.hh>

namespace syntax {

    Ignore::Ignore () {}

    Expression Ignore::init (const Ignore & alloc) {
	auto ret = new (Z0) Ignore ();
	ret-> _token = alloc._token;
	return Expression {ret};
    }

    Expression Ignore::init (const lexing::Word & location) {
	auto ret = new (Z0) Ignore ();
	ret-> _token = location;
	return Expression {ret};
    }

    Expression Ignore::clone () const {
	return Ignore::init (*this);
    }

    bool Ignore::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Ignore thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Ignore::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Ignore>", i, '\t');
    }
    
}
