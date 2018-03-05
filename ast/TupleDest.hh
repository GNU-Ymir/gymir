#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace semantic {
    class ISymbol;
    typedef ISymbol* Symbol;
}

namespace syntax {

    class ITupleDest : public IInstruction {

	std::vector <Var> decls;
	std::vector <Expression> insts;
	Expression right;
	bool isVariadic;

    public:

	ITupleDest (Word token, bool isVariadic, std::vector <Var> decls, Expression right);

	ITupleDest (Word token, std::vector <Expression> insts, Expression right);
	
	Instruction instruction () override;

	Ymir::Tree toGeneric () override;
	
	Instruction templateReplace (const std::map <std::string, Expression>&) override;	

	std::vector <semantic::Symbol> allInnerDecls () override;
	
	void print (int nb = 0) override;
	
	virtual ~ITupleDest ();

    private :

	Instruction Incompatible (semantic::Symbol info);
	
    };

    typedef ITupleDest* TupleDest;    
}
