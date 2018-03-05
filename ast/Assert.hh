#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IAssert : public IInstruction {

	Expression expr;

	Expression msg;

    public :

	IAssert (Word token, Expression test, Expression msg, bool isStatic = false);
	
	Instruction instruction () override;

	Instruction templateReplace (const std::map <std::string, Expression>&) override;

	std::vector <semantic::Symbol> allInnerDecls () override;

	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual ~IAssert ();

    private :

	Ymir::Tree callPrint (Ymir::Tree);
	
    };

    typedef IAssert* Assert;
    
}
