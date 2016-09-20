#pragma once

#include "Instruction.hh"
#include "Expression.hh"

namespace Syntax {
    
    struct Delete : Instruction {
	ExpressionPtr elem;
	Delete (Lexical::TokenPtr token, ExpressionPtr elem)
	    : Instruction (token, AstEnums::DELETE),
	      elem (elem)
	{}
	
	virtual void print (int nb = 0);
    };

};
