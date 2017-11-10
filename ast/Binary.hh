#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IBinary : public IExpression {

	Expression left, right;
	
	bool isRight = false;

    public :

	IBinary (Word word, Expression left, Expression right) :
	    IExpression (word),
	    left (left),
	    right (right)
	{
	    if (this-> left) this-> left-> inside = this;
	    if (this-> right) this-> right-> inside = this;	    
	}

	void print (int nb = 0) override {
	    printf ("\n%*c<Binary> : %s",
		      nb, ' ',
		    this-> token.toString ().c_str ()
	    );	
	    
	    this-> left-> print (nb + 4);
	    this-> right-> print (nb + 4);
	}	
	
    };

    typedef IBinary* Binary;
    
}
