#include <ymir/syntax/expression/Return.hh>

namespace syntax {

    Return::Return () : _value (Expression::empty ())
    {}

    Expression Return::init (const Return & aux) {
	auto ret = new (Z0) Return ();
	ret-> _location = aux._location;
	ret-> _value = aux._value;
	return Expression {ret};
    }

    Expression Return::init (const lexing::Word & location, const Expression & value) {
	auto ret = new (Z0) Return ();
	ret-> _location = location;
	ret-> _value = value;
	return Expression {ret};
    }

    Expression Return::clone () const {
	return Return::init (*this);
    }

    bool Return::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Return thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Return::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Return> ", i, '\t');
	this-> _value.treePrint (stream, i + 1);
    }
    
}
