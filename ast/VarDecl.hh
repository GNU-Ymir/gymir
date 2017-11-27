#pragma once

#include "Var.hh"
#include "Expression.hh"
#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IVar;
    typedef IVar* Var;
    
    class IVarDecl : public IInstruction {
	std::vector <Var> decls;
	std::vector <Expression> insts;
	std::vector <Word> decos;

    public:

	IVarDecl (Word word, std::vector <Word> decos, std::vector <Var> decls, std::vector <Expression> insts);
	
	Instruction instruction () override;	
	
	void print (int nb = 0) override;

    private:

	IVarDecl (Word word);
	
    };

    typedef IVarDecl* VarDecl;
    
}
