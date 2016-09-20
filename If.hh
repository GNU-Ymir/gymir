#pragma once

#include "Instruction.hh"
#include "Block.hh"
#include "Expression.hh"

namespace Syntax {
    
    struct If : Instruction {
	ExpressionPtr test;
	BlockPtr block;
	InstructionPtr else_bl;
	If (Lexical::TokenPtr token, ExpressionPtr test, BlockPtr block)
	    : Instruction (token, AstEnums::IF),
	      test (test),
	      block (block),
	      else_bl (NULL)
	{}

	If (Lexical::TokenPtr token, ExpressionPtr test, BlockPtr block, InstructionPtr else_bl)
	    : Instruction (token, AstEnums::IF),
	      test (test),
	      block (block),
	      else_bl (else_bl)
	{}
	
	virtual void print (int nb = 0);
    };        


};
