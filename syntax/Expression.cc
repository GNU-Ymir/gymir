#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/ExpressionWrapper.hh>

namespace syntax {

    Expression::Expression (IExpression * expr) : Proxy<IExpression, Expression> (expr)
    {}

    Expression Expression::empty () {
	return Expression {nullptr};
    }

    Declaration Expression::toDeclaration (const Expression & value) {
	return ExpressionWrapper::init (value);
    }
    
    bool IExpression::isOf (const IExpression *) const {
	return false;
    }

    IExpression::~IExpression () {}


    
}
