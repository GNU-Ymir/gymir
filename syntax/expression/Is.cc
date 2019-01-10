#include <ymir/syntax/expression/Is.hh>

namespace syntax {

    Is::Is () : _element (Expression::empty ()) {}

    Expression Is::init (const Is & op) {
	auto ret = new (Z0) Is ();
	ret-> _location = op._location;
	ret-> _element = op._element;
	ret-> _params = op._params;
	return Expression {ret};
    }

    Expression Is::init (const lexing::Word & location, const Expression & element, const std::vector <Expression> & params) {
	auto ret = new (Z0) Is ();
	ret-> _location = location;
	ret-> _element = element;
	ret-> _params = params;
	return Expression {ret};
    }

    Expression Is::clone () const {
	return Is::init (*this);
    }

    bool Is::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Is thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    
}
