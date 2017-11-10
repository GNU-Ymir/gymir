#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IExpand : public IExpression {
	Expression expr;

    public :

	IExpand (Word begin, Expression expr) :
	    IExpression (begin),
	    expr (expr)
	{}

	void print (int nb = 0) override {
	    printf("\n%*c<Expand> %s",
		   nb, ' ',
		   this-> token.toString ().c_str ()
	    );
	    this-> expr-> print (nb + 4);
	}
		
    };

    typedef IExpand* Expand;
    
}
