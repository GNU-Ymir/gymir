#pragma once

#include "Syntax.hh"
#include "Lexer.hh"

namespace Syntax {

    struct Instruction : Ast {
	Instruction (Lexical::TokenPtr ptr) : Ast (ptr) {}
    };

    typedef std::tr1::shared_ptr<Instruction> InstructionPtr;

    struct Block : Instruction {
	std::vector <InstructionPtr> instructions;

	Block (Lexical::TokenPtr ptr, std::vector <InstructionPtr> insts)
	    : Instruction (ptr),
	      instructions (insts) {}
    };
    
    typedef std::tr1::shared_ptr<Block> BlockPtr;
    
    
};
