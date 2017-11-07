#pragma once

#include "Instruction.hh"
#include "Declaration.hh"
#include "Tree.hh"

namespace Syntax {
       
    struct Block : Instruction {
	std::vector<DeclarationPtr> decls;
	std::vector <InstructionPtr> instructions;	

	Block (Lexical::TokenPtr ptr, std::vector<DeclarationPtr> decls, std::vector <InstructionPtr> insts)
	    : Instruction (ptr, AstEnums::BLOCK),
	      decls (decls),
	      instructions (insts)
	{}
	
	Instruction * instruction () override;

	Ymir::Tree toGeneric (bool);
	       
	virtual void print (int nb = 0);
	
    };
    
    typedef Block* BlockPtr;

};
