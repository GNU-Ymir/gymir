#pragma once

#include "Instruction.hh"

namespace Syntax {
    
    struct ElseIf : Instruction {
	InstructionPtr if_bl;
	ElseIf (Lexical::TokenPtr token, InstructionPtr if_bl)
	    : Instruction (token, AstEnums::ELSEIF),
	      if_bl (if_bl)
	{}
	
	virtual void print (int nb = 0);
    };

};
