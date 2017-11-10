#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IIf;
    typedef IIf* If;
    
    class IIf : public IInstruction {
	
	Expression test;
	Block block;
	If else_;

    public:

	IIf (Word word, Expression test, Block block, bool isStatic = false) :
	    IInstruction (word),
	    test (test),
	    block (block),
	    else_ (NULL)
	{
	    if (this-> test)
		this-> test-> inside = this;
	    this-> isStatic = isStatic;
	}

	IIf (Word word, Expression test, Block block, If else_, bool isStatic = false) : 
	    IInstruction (word),
	    test (test),
	    block (block),
	    else_ (else_)
	{
	    if (this-> test)
		this-> test-> inside = this;
	    this-> isStatic = isStatic;
	    if (this-> else_)
		this-> else_-> isStatic = isStatic;
	}

	void print (int nb = 0) override {
	    if (this-> test) {
		printf ("\n%*c<%sIf> %s",			
			nb, ' ',
			this-> isStatic ? "static_" : "",
			this-> token.toString ().c_str ()
		);		
		this-> test-> print (nb + 4);		
	    } else {
		printf ("\n%*c<%sElse> %s",
			nb, ' ',
			this-> isStatic ? "static_" : "",
			this-> token.toString ().c_str ()
		);
	    }

	    this-> block-> print (nb + 4);
	    if (this-> else_)
		this-> else_-> print (nb + 8);	    
	}
	       
    };
    
}
