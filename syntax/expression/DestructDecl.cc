#include <ymir/syntax/expression/DestructDecl.hh>
#include <algorithm>

namespace syntax {

    DestructDecl::DestructDecl () :
	IExpression (lexing::Word::eof ()),
	_params ({}),
	_value (Expression::empty ())
    {}
    
    DestructDecl::DestructDecl (const lexing::Word & loc, const std::vector <Expression> & params, const Expression & value, bool isVariadic) :
	IExpression (loc),
	_params (params),
	_value (value),
	_isVariadic (isVariadic)
    {}

    Expression DestructDecl::init (const lexing::Word & loc, const std::vector <Expression> & params, const Expression & value, bool isVariadic) {
	return Expression {new (Z0) DestructDecl (loc, params, value, isVariadic)};
    }

    bool DestructDecl::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	DestructDecl thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void DestructDecl::treePrint (Ymir::OutBuffer & stream, int i) const {       
	stream.writef ("%*<DestructDecl> : ", i, '\t');
	for (auto & it : this-> _params)
	    it.treePrint (stream, i + 1);
	this-> _value.treePrint (stream, i + 2);	
    }

    const std::vector <Expression> & DestructDecl::getParameters () const {
	return this-> _params;
    }
    
    const Expression & DestructDecl::getValue () const {
	return this-> _value;
    }

    bool DestructDecl::isVariadic () const {
	return this-> _isVariadic;
    }
    
}
