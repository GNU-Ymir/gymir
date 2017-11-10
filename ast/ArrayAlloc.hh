#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IArrayAlloc : public IExpression {

	Expression type;
	
	Expression size;
	
	//Semantic::InfoType cster;

    public:

	IArrayAlloc (Word token, Expression type, Expression size) :
	    IExpression (token),
	    type (type),
	    size (size)
	{
	    this-> size-> inside = this;
	    if (this-> type)
		this-> type-> inside = this;
	}
    
	void print (int nb = 0) override {
	    printf ("\n%*c<ArrayAlloc> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    
	    if (this-> type)
		this-> type-> print (nb + 4);
	    this-> size-> print (nb + 4);
	}	
    };

    typedef IArrayAlloc* ArrayAlloc;
}
