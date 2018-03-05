#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {

    class IIf;
    typedef IIf* If;
    
    class IIf : public IInstruction {
	
	Expression test;
	Block block;
	If else_;
	semantic::InfoType info;
	
    public:

	IIf (Word word, Expression test, Block block, bool isStatic = false);
	
	IIf (Word word, Expression test, Block block, If else_, bool isStatic = false);

	Instruction instruction () override;	

	Instruction templateReplace (const std::map <std::string, Expression>&) override;
	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	
    };
    
}
