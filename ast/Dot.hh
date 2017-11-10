#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IDot : public IExpression {
	
	Expression left, right;

    public:

	IDot (Word word, Expression left, Expression right) :
	    IExpression (word),
	    left (left),
	    right (right)
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<Dot> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> left-> print (nb + 4);
	    this-> right-> print (nb + 4);
	}
	
    };

    typedef IDot* Dot;
}
