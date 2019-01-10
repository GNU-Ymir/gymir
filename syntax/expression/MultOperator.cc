#include <ymir/syntax/expression/MultOperator.hh>

namespace syntax {

    MultOperator::MultOperator () : _element (Expression::empty ()) {}

    Expression MultOperator::init (const MultOperator & op) {
	auto ret = new (Z0) MultOperator ();
	ret-> _location = op._location;
	ret-> _end = op._end;
	ret-> _element = op._element;
	ret-> _params = op._params;
	return Expression {ret};
    }

    Expression MultOperator::init (const lexing::Word & location, const lexing::Word & end, const Expression & element, const std::vector <Expression> & params) {
	auto ret = new (Z0) MultOperator ();
	ret-> _location = location;
	ret-> _end = end;
	ret-> _element = element;
	ret-> _params = params;
	return Expression {ret};
    }

    Expression MultOperator::clone () const {
	return MultOperator::init (*this);
    }

    bool MultOperator::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MultOperator thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    
}
