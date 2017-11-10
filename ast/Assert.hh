#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IAssert : public IInstruction {

	Expression expr;

	Expression msg;

    public :

	IAssert (Word token, Expression test, Expression msg, bool isStatic = false) :
	    IInstruction (token),
	    expr (test),
	    msg (msg)
	{
	    this-> expr-> inside = this;
	    if (this-> msg)
		this-> msg-> inside = this;
	    this-> isStatic = isStatic;
	}


	void print (int nb = 0) override {
	    printf ("\n%*c<Assert> %s",
		      nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    
	    this-> expr-> print (nb + 4);
	    if (this-> msg) this-> msg-> print (nb + 4);
	}	
    };

    typedef IAssert* Assert;
    
}
