#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IFor : public IInstruction {

	Word id;
	std::vector <Var> var;
	Expression iter;
	Block block;
	// InfoType ret;
	
    public:

	IFor (Word token, Word id, std::vector <Var> var, Expression iter, Block bl);

	Instruction instruction () override;
	
	Instruction templateReplace (std::map <std::string, Expression>) override;
	
	void print (int nb = 0) override;		
    };

    typedef IFor* For;
    
}
