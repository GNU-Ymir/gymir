#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    typedef unsigned char ubyte;
    
    class IConstRange : public IExpression {
	Expression left, right;
	//Semantic::InfoType content;
	ubyte lorr = 0;
	//Semantic::InfoType caster = NULL;

    public:
	
	IConstRange (Word token, Expression left, Expression right) :
	    IExpression (token),
	    left (left),
	    right (right)
	{
	    this-> left-> inside = this;
	    this-> right-> inside = this;
	}

	void print (int nb = 0) override {
	    printf ("\n%*c<ConstRange> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> left-> print (nb + 4);
	    this-> right-> print (nb + 4);
	}
    };

    typedef IConstRange* ConstRange;
}
