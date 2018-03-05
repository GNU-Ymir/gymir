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
	semantic::InfoType ret;
	
    public:

	IFor (Word token, Word id, const std::vector <Var>& var, Expression iter, Block bl);

	Instruction instruction () override;
	
	Instruction templateReplace (const std::map <std::string, Expression>&) override;

	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual ~IFor ();
	
    };

    typedef IFor* For;
    
}
