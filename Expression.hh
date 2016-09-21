#pragma once

#include "Instruction.hh"
#include "TypeInfo.hh"

namespace Syntax {

    struct Expression : Instruction {
	Expression (Lexical::TokenPtr ptr, AstEnums::AstEnum type)
	    : Instruction (ptr, type)
	{}
	
	Expression (Lexical::TokenPtr ptr)
	    : Instruction (ptr, AstEnums::EXPRESSION)
	{}

	Semantic::TypeInfo * getType () const {
	    return this->info;
	}
	
	virtual void print (int nb = 0);	
	
    protected:
	
	Semantic::TypeInfo * info = NULL;
	
    };

    typedef Expression* ExpressionPtr;

}
