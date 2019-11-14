#include <ymir/syntax/expression/DestructDecl.hh>
#include <algorithm>

namespace syntax {

    DestructDecl::DestructDecl () :
	IExpression (lexing::Word::eof ()),
	_params ({}),
	_value (Expression::empty ())
    {}
    
    DestructDecl::DestructDecl (const lexing::Word & loc) :
	IExpression (loc),
	_params ({}),
	_value (Expression::empty ())
    {}

    Expression DestructDecl::init (const lexing::Word & loc, const std::vector <Expression> & params, const Expression & value, bool isVariadic) {
	auto ret = new (Z0) DestructDecl (loc);
	ret-> _params = params;
	ret-> _value = value;
	ret-> _isVariadic = isVariadic;
	return Expression {ret};
    }

    Expression DestructDecl::clone () const {
	return Expression {new DestructDecl (*this)};
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
