#pragma once

#include "Ast.hh"

namespace Syntax {

    struct Instruction : Ast {

	Instruction (Lexical::TokenPtr ptr, AstEnums::AstEnum type)
	    : Ast (ptr, type)
	{}

	Instruction (Lexical::TokenPtr ptr)
	    : Ast (ptr, AstEnums::INSTRUCTION)
	{}
	
	virtual void print (int nb = 0);
    };    

    typedef Instruction* InstructionPtr;

};
