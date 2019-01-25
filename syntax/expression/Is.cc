#include <ymir/syntax/expression/Is.hh>

namespace syntax {

    Is::Is () :
	IExpression (lexing::Word::eof ()),
	_element (Expression::empty ())
    {}
    
    Is::Is (const lexing::Word & loc) :
	IExpression (loc),
	_element (Expression::empty ())
    {}

    Expression Is::init (const lexing::Word & location, const Expression & element, const std::vector <Expression> & params) {
	auto ret = new (Z0) Is (location);
	ret-> _element = element;
	ret-> _params = params;
	return Expression {ret};
    }

    Expression Is::clone () const {
	return Expression {new Is (*this)};
    }

    bool Is::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Is thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    
}
