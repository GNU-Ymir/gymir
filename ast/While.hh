#pragma once

#include "Instruction.hh"
#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IWhile : public IInstruction {

	Word name;
	Expression test;
	Block block;

    public:

	IWhile (Word token, Word name, Expression test, Block block) :
	    IInstruction (token),
	    name (name),
	    test (test),
	    block (block)
	{}

	IWhile (Word token, Expression test, Block block) :
	    IInstruction (token),
	    test (test),
	    block (block)
	{}

	Instruction instruction () override {
	    Ymir::Error::assert ("TODO");
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<While> %s:%s",
		    nb, ' ',
		    this-> name.isEof () ? "_" : this-> name.getStr ().c_str (),
		    this-> token.toString ().c_str ()
	    );

	    this-> test-> print (nb + 4);
	    this-> block-> print (nb + 4);	    
	}
	
    };

    typedef IWhile* While;
    
}
