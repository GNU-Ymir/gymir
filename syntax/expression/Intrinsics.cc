#include <ymir/syntax/expression/Intrinsics.hh>

namespace syntax {

    Intrinsics::Intrinsics () :
	_value (Expression::empty ())
    {}

    Expression Intrinsics::init (const Intrinsics & alloc) {
	auto ret = new (Z0) Intrinsics ();
	ret-> _op = alloc._op;
	ret-> _value = alloc._value;
	return Expression {ret};
    }

    Expression Intrinsics::init (const lexing::Word & location, const Expression & value) {
	auto ret = new (Z0) Intrinsics ();
	ret-> _op = location;
	ret-> _value = value;
	return Expression {ret};
    }

    Expression Intrinsics::clone () const {
	return Intrinsics::init (*this);
    }

    bool Intrinsics::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Intrinsics thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    

    void Intrinsics::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Intrisics> ", i, '\t');
	stream.writeln (this-> _op);
	this-> _value.treePrint (stream, i + 1);
    }

}
