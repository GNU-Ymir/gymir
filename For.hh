#pragma once

#include "Instruction.hh"
#include <vector>
#include "Expression.hh"
#include "Block.hh"

namespace Syntax {
    
    struct For : Instruction {
	std::vector<ExpressionPtr> inits;
	ExpressionPtr test;
	std::vector<ExpressionPtr> iter;
	BlockPtr block;
	For (Lexical::TokenPtr token, std::vector<ExpressionPtr> inits, ExpressionPtr test, std::vector<ExpressionPtr> iter, BlockPtr block) 
	    : Instruction (token, AstEnums::FOR),
	      inits (inits),
	      test (test),
	      iter (iter),
	      block (block)
	{}
	
	virtual void print (int nb = 0);
    };
    
};
