#include <ymir/syntax/expression/For.hh>

namespace syntax {

    For::For () :
	_iter (Expression::empty ()),
	_block (Expression::empty ())
    {}

    Expression For::init (const For & for_) {
	auto ret = new (Z0) For ();
	ret-> _location = for_._location;
	ret-> _vars = for_._vars;
	ret-> _iter = for_._iter;
	ret-> _block = for_._block;
	return Expression {ret};
    }

    Expression For::init (const lexing::Word & location, const std::vector <Expression> & vars, const Expression & iter, const Expression & block) {
	auto ret = new (Z0) For ();
	ret-> _location = location;
	ret-> _vars = vars;
	ret-> _iter = iter;
	ret-> _block = block;
	return Expression {ret};
    }

    Expression For::clone () const {
	return For::init (*this);
    }

    bool For::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	For thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }
    

}
