#pragma once

#include "Var.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IOfVar : public IVar {

	Var type;

    public:

	IOfVar (Word ident, Var type) :
	    IVar (ident),
	    type (type)
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<OfVar> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> type-> print (nb + 4);
	}
	
    };


    class IVariadicVar : public IVar {
    public:

	IVariadicVar (Word ident) : IVar (ident) {}

	void print (int nb = 0) override {
	    printf ("\n%*c<VariadicVar> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	}
    };

    typedef IOfVar* OfVar;
    typedef IVariadicVar* VariadicVar;
        
}
