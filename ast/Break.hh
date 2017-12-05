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
	
	IBreak (Word token);

	IBreak (Word token, Word ident);

	Instruction instruction () override;
	
	void print (int nb = 0) override;	
    };

    typedef IBreak* Break;
}
