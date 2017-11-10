#pragma once

#include "Instruction.hh"

#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IExpression : public IInstruction {
	// Symbol info;
    public:
	Instruction inside;

	IExpression (Word word) : IInstruction (word) {
	}

	virtual void print (int nb = 0) override = 0;
	
    };

    typedef IExpression* Expression;
    
}
