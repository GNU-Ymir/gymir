#pragma once

#include "Instruction.hh"
#include "Block.hh"

namespace Syntax {
    
    struct Else : Instruction {
	BlockPtr block;
	Else (Lexical::TokenPtr token, BlockPtr block)
	    : Instruction (token, AstEnums::ELSE),
	      block (block)
	{}
	
	virtual void print (int nb = 0);
    };


};
