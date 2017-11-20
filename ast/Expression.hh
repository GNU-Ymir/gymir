#pragma once

#include "Instruction.hh"

#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include "../semantic/pack/Symbol.hh"

namespace syntax {

    class IExpression : public IInstruction {
    public:
	
	::semantic::Symbol info;
	
	Instruction inside;

	IExpression (Word word) :
	    IInstruction (word)
	{}

	virtual void print (int) override {}
	
    };

    typedef IExpression* Expression;
    
}
