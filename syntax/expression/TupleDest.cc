#include <ymir/syntax/expression/TupleDest.hh>

namespace syntax {

    TupleDest::TupleDest () :
	_value (Expression::empty ())
    {}

    Expression TupleDest::init (const TupleDest & dest) {
	auto ret = new (Z0) TupleDest ();
	ret-> _location = dest._location;
	ret-> _vars = dest._vars;
	ret-> _value = dest._value;
	ret-> _isVariadic = dest._isVariadic;
	return Expression {ret};
    }
    
    Expression TupleDest::init (const lexing::Word & location, const std::vector <Expression> & vars, const Expression & value, bool isVariadic) {
	auto ret = new (Z0) TupleDest ();
	ret-> _location = location;
	ret-> _vars = vars;
	ret-> _value = value;
	ret-> _isVariadic = isVariadic;
	return Expression {ret};
    }

    Expression TupleDest::clone () const {
	return TupleDest::init (*this);
    }

    Expression TupleDest::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	TupleDest thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }    
    
}
