#include <ymir/syntax/expression/Dollar.hh>

namespace syntax {

    Dollar::Dollar () {}

    Expression Dollar::init (const Dollar & alloc) {
	auto ret = new (Z0) Dollar ();
	ret-> _token = alloc._token;
	return Expression {ret};
    }

    Expression Dollar::init (const lexing::Word & location) {
	auto ret = new (Z0) Dollar ();
	ret-> _token = location;
	return Expression {ret};
    }

    Expression Dollar::clone () const {
	return Dollar::init (*this);
    }

    bool Dollar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Dollar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Dollar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Dollar>", i, '\t');
    }
    
}
