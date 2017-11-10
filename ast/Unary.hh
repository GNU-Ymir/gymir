#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IUnary : public IExpression {
	Expression elem;

    public:

	IUnary (Word word, Expression elem) :
	    IExpression (word),
	    elem (elem)
	{
	    this-> elem-> inside = this;
	}

	void print (int nb = 0) override {
	    printf ("\n%*c<Unary> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> elem-> print (nb + 4);
	}
	
    };

    typedef IUnary* Unary;

}
