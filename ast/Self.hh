#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class ISelf : public IDeclaration {

	Word ident;
	Block block;

    public:

	ISelf (Word ident, Block block) :
	    ident (ident),
	    block (block)
	{}

	void declare () override {}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Self> %s",
		    nb, ' ',
		    this-> ident.toString ().c_str ()
	    );
	    
	    this-> block-> print (nb + 4);
	}	

    };

    typedef ISelf* Self;
    
}
