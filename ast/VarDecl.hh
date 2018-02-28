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

	std::vector <Var> statics;
	std::vector <Expression> staticExprs;
	std::vector <Word> decos;

    public:

	IVarDecl (Word word, const std::vector <Word>& decos, const std::vector <Var> &decls, const std::vector <Expression>& insts);
	
	Instruction instruction () override;	

	Instruction templateReplace (const std::map <std::string, Expression>&) override;	
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual ~IVarDecl ();
	
    private:

	IVarDecl (Word word);
	
    };

    typedef IVarDecl* VarDecl;
    
}
