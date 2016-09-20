#pragma once

#include "Instruction.hh"
#include "Expression.hh"

namespace Syntax {
    
    struct Return : Instruction {
	ExpressionPtr ret;
	Return(Lexical::TokenPtr token)
	    : Instruction (token, AstEnums::RETURN)
	{}

	Return (Lexical::TokenPtr token, ExpressionPtr ret)
	    : Instruction (token, AstEnums::RETURN),
	      ret (ret)
	{}
	
	virtual void print (int nb = 0);
    };


};
