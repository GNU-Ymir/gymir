#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IBreak : public IInstruction {

	Word ident;
	ulong nbBlock;

    public:
	
	IBreak (Word token) : IInstruction (token) {
	    this-> ident.setEof ();
	}

	IBreak (Word token, Word ident) :
	    IInstruction (token),
	    ident (ident) {
	}

	Instruction instruction () override {
	    Ymir::Error::assert ("TODO");
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Break> %s",
		      nb, ' ',
		      this-> token.toString ().c_str ()
	    );		  	    
	}
	
    };

    typedef IBreak* Break;
}
