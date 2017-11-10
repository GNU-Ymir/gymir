#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IDColon : public IExpression {

	Expression left, right;

    public:

	IDColon (Word token, Expression left, Expression right) :
	    IExpression (token),
	    left (left),
	    right (right)
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<DColon> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> left-> print (nb + 4);
	    this-> right-> print (nb + 4);
	}

    };
    
};
