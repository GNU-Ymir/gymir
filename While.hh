#pragma once

#include "Instruction.hh"
#include "Expression.hh"
#include "Block.hh"

namespace Syntax {

    struct While : Instruction {
	ExpressionPtr test;
	BlockPtr block;
	While (Lexical::TokenPtr token, ExpressionPtr test, BlockPtr block)
	    : Instruction (token, AstEnums::WHILE),
	      test (test),
	      block (block)
	{}
	
	virtual void print (int nb = 0);
	
    };
    
};
