#pragma once

#include "Syntax.hh"
#include "Lexer.hh"

namespace Syntax {

    struct Instruction : Ast {
	Instruction (Lexical::TokenPtr ptr) : Ast (ptr) {}
    };

    typedef Instruction* InstructionPtr;

    struct Block : Instruction {
	std::vector <InstructionPtr> instructions;

	Block (Lexical::TokenPtr ptr, std::vector <InstructionPtr> insts)
	    : Instruction (ptr),
	      instructions (insts) {}
    };
    
    typedef Block* BlockPtr;
    
    
};
