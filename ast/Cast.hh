#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class ICast : public IExpression {

	Expression type;
	Expression expr;
	//Token deco;

    public:

	ICast (Word begin, Expression type, Expression expr) :
	    IExpression (begin),
	    type (type),
	    expr (expr) {
	    this-> type-> inside = this;
	    this-> expr-> inside = this;
	}

	void print (int nb = 0) override {
	    printf ("\n%*c<Cast> %s", nb, ' ', this-> token.toString ().c_str ());
	    this-> type-> print (nb + 4);
	    this-> expr-> print (nb + 4);
	}
	
    };

    typedef ICast* Cast;    
}
