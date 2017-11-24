#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IReturn : public IInstruction {

	Expression elem;

    public:

	IReturn (Word ident) : IInstruction (ident), elem (NULL) {}
	
	IReturn (Word ident, Expression elem) :
	    IInstruction (ident),
	    elem (elem)
	{}

	Instruction instruction () override {
	    Ymir::Error::assert ("TODO");
	}

	
	void print (int nb = 0) override {
	    printf ("\n%*c<Return> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    
	    if (this-> elem)
		this-> elem-> print (nb + 4);
	}
	
    };
    
}
