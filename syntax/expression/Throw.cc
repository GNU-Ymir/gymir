#include <ymir/syntax/expression/Throw.hh>

namespace syntax {

    Throw::Throw () : _value (Expression::empty ())
    {}

    Expression Throw::init (const Throw & thr) {
	auto ret = new (Z0) Throw ();
	ret-> _location = thr._location;
	ret-> _value = thr._value;
	return Expression {ret};
    }

    Expression Throw::init (const lexing::Word & location, const Expression & value) {
	auto ret = new (Z0) Throw ();
	ret-> _location = location;
	ret-> _value = value;
	return Expression {ret};
    }

    Expression Throw::clone () const {
	return Throw::init (*this);
    }

    bool Throw::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Throw thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

}
