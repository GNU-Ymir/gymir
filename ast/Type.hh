#pragma once

#include "Var.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IType : public IVar {
    public:

	IType (Word token/*, InfoType type*/) :
	    IVar (token)
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<Type> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	}
	
    };

    typedef IType* Type;
    
}
