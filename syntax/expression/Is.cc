#include <ymir/syntax/expression/Is.hh>

namespace syntax {

    Is::Is () :
	IExpression (lexing::Word::eof ()),
	_element (Expression::empty ())
    {}
    
    Is::Is (const lexing::Word & loc, const Expression & element, const std::vector <Expression> & params) :
	IExpression (loc),
	_element (element),
	_params (params)
    {}

    Expression Is::init (const lexing::Word & location, const Expression & element, const std::vector <Expression> & params) {
	return Expression {new (Z0) Is (location, element, params)};
    }

    bool Is::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Is thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    
}
