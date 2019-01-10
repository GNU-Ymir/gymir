#include <ymir/syntax/expression/Char.hh>

namespace syntax {

    Char::Char () {}

    Expression Char::init (const Char & alloc) {
	auto ret = new (Z0) Char ();
	ret-> _location = alloc._location;
	ret-> _sequence = alloc._sequence;
	return Expression {ret};
    }

    Expression Char::init (const lexing::Word & location, const std::string & sequence) {
	auto ret = new (Z0) Char ();	
	ret-> _location = location;
	ret-> _sequence = sequence;
	return Expression {ret};
    }

    Expression Char::clone () const {
	return Char::init (*this);
    }

    bool Char::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Char thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    
}
