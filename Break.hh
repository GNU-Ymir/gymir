#pragma once

#include "Instruction.hh"

namespace Syntax {

    struct Break : Instruction {
	Break (Lexical::TokenPtr token)
	    : Instruction (token, AstEnums::BREAK)
	{}
	
	virtual void print (int nb = 0);
    };

};
