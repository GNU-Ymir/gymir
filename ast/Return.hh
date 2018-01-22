#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IReturn : public IInstruction {

	Expression elem;
	semantic::InfoType caster;
	
    public:

	IReturn (Word ident);
	
	IReturn (Word ident, Expression elem);

	Instruction instruction () override;

	Instruction templateReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;
	
    };
    
}
