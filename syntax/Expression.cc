#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/ExpressionWrapper.hh>

namespace syntax {

    Expression::Expression (IExpression * expr) : Proxy<IExpression, Expression> (expr)
    {}

    Expression Expression::empty () {
	return Expression {nullptr};
    }

    bool Expression::isEmpty () {
	return this-> _value == nullptr;
    }
    
    Declaration Expression::toDeclaration (const Expression & value) {
	return ExpressionWrapper::init (value);
    }
    
    void Expression::treePrint (Ymir::OutBuffer & stream, int i)  const {	
	if (this-> _value == NULL) {
	    stream.writef ("%*", i, '\t');
	    stream.writeln ("<null>");
	} else this-> _value-> treePrint (stream, i);
    }

    bool IExpression::isOf (const IExpression *) const {
	return false;
    }

    void IExpression::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<TODO>", i, '\t');
    }
    
    
    IExpression::~IExpression () {}


    
}
