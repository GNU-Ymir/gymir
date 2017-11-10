#pragma once

#include "Var.hh"
#include "Expression.hh"
#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IVar;
    typedef IVar* Var;
    
    class IVarDecl : public IInstruction {
	std::vector <Var> decls;
	std::vector <Expression> insts;
	std::vector <Word> decos;

    public:

	IVarDecl (Word word, std::vector <Word> decos, std::vector <Var> decls, std::vector <Expression> insts) :
	    IInstruction (word),
	    decls (decls),
	    insts (insts),
	    decos (decos)
	{}

	void print (int nb = 0) override;	
    };

    typedef IVarDecl* VarDecl;
    
}
