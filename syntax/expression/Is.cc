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
	return Expression {new (NO_GC) Is (location, element, params)};
    }

    std::string Is::prettyString () const {
	return "";
    }
    
    
}
