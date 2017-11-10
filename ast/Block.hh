#pragma once

#include "Instruction.hh"
#include <vector>
#include <stdio.h>
#include "../errors/_.hh"
#include "../semantic/_.hh"
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
	
	void print (int nb = 0) override; 	
    };

    typedef IBlock* Block;
    
}
