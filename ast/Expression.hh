#pragma once

#include "Instruction.hh"

#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include "../semantic/pack/Symbol.hh"

namespace syntax {

    class IExpression : public IInstruction {
    protected:
	
	::semantic::Symbol _info;
	
    public:
	Instruction inside;

	IExpression (Word word) :
	    IInstruction (word)
	{}

	virtual void print (int) override {}

	::semantic::Symbol& info () {
	    return this-> _info;
	}
	
    };

    typedef IExpression* Expression;
    
}
