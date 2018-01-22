#pragma once

#include "Instruction.hh"
#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IWhile : public IInstruction {

	Word name;
	Expression test;
	Block block;

    public:

	IWhile (Word token, Word name, Expression test, Block block);

	IWhile (Word token, Expression test, Block block);

	Instruction instruction () override;

	Instruction templateReplace (const std::map <std::string, Expression>&) override;	
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;	
    };

    typedef IWhile* While;
    
}
