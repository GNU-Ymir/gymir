#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IAssert : public IInstruction {

	Expression expr;

	Expression msg;

    public :

	IAssert (Word token, Expression test, Expression msg, bool isStatic = false);	

	Instruction instruction () override;
	
	void print (int nb = 0) override;
	
    };

    typedef IAssert* Assert;
    
}
