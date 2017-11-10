#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IGlobal : public IDeclaration {

	Word ident;
	Expression expr, type;

    public:

	IGlobal (Word ident, Expression expr, Expression type = NULL) :
	    ident (ident),
	    expr (expr),
	    type (type)
	{}

	void print (int nb = 0) override {
	    printf ("\n%*c<Global> %s",
		    nb, ' ',
		    this-> ident.toString ().c_str ()
	    );

	    if (this-> expr)
		this-> expr-> print (nb + 4);
	    
	    if (this-> type)
		this-> type-> print (nb + 4);
	}
	
    };

    typedef IGlobal* Global;
    
}
