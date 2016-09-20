#pragma once

#include "Instruction.hh"

namespace Syntax {

    struct Expression : Instruction {
	Expression (Lexical::TokenPtr ptr, AstEnums::AstEnum type)
	    : Instruction (ptr, type)
	{}
	
	Expression (Lexical::TokenPtr ptr)
	    : Instruction (ptr, AstEnums::EXPRESSION)
	{}
	
	virtual void print (int nb = 0);
    };

    typedef Expression* ExpressionPtr;

}
