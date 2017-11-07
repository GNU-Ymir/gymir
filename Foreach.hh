#pragma once

#include "Instruction.hh"
#include "Expression.hh"
#include "Block.hh"

namespace Syntax {
    
    struct Foreach : Instruction {
	ExpressionPtr var;
	ExpressionPtr iter;
	BlockPtr block ;
	Foreach (Lexical::TokenPtr token, ExpressionPtr var, ExpressionPtr iter, BlockPtr block)
	    : Instruction (token, AstEnums::FOREACH),
	      var (var),
	      iter (iter),
	      block (block)
	{}
	
	virtual void print (int nb = 0);
    };

};
