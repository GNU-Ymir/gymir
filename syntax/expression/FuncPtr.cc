#include <ymir/syntax/expression/FuncPtr.hh>

namespace syntax {

    FuncPtr::FuncPtr () : _ret (Expression::empty ()) {}

    Expression FuncPtr::init (const FuncPtr & op) {
	auto ret = new (Z0) FuncPtr ();
	ret-> _location = op._location;
	ret-> _ret = op._ret;
	ret-> _params = op._params;
	return Expression {ret};
    }

    Expression FuncPtr::init (const lexing::Word & location, const Expression & retType, const std::vector <Expression> & params) {
	auto ret = new (Z0) FuncPtr ();
	ret-> _location = location;
	ret-> _ret = retType;
	ret-> _params = params;
	return Expression {ret};
    }

    Expression FuncPtr::clone () const {
	return FuncPtr::init (*this);
    }

    bool FuncPtr::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	FuncPtr thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    
}
