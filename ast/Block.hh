#pragma once

#include "Instruction.hh"
#include <ymir/utils/Array.hh>
#include <stdio.h>
#include "../errors/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IDeclaration;
    typedef IDeclaration* Declaration;
    
    class IBlock : public IInstruction {

	Word ident;
	std::vector <Declaration> decls;
	std::vector <Instruction> insts;

    public :

	IBlock (Word word, std::vector <Declaration> decls, std::vector <Instruction> insts) :
	    IInstruction (word),
	    decls (decls),
	    insts (insts)
	{
	    this-> ident.setEof ();
	}

	Word& getIdent ();

	Instruction instruction () override;

	Instruction templateReplace (const std::map <std::string, Expression>&) override;
	
	IBlock* block ();

	IBlock* blockWithoutEnter ();
	
	Ymir::Tree toGeneric () override;

	Ymir::Tree toGenericSimple ();
	
	void print (int nb = 0) override;

	virtual ~IBlock ();
	
    };

    typedef IBlock* Block;
    
}
