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
	std::vector <bool> _const;
	semantic::InfoType ret;
	
    public:

	IFor (Word token, Word id, const std::vector <Var>& var, Expression iter, Block bl, std::vector <bool> _const);

	Instruction instruction () override;
	
	Instruction templateReplace (const std::map <std::string, Expression>&) override;

	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual ~IFor ();

    private :

	Instruction immutable (Expression expr);

	Instruction immutableRange (std::vector <Var> & vars, Expression expr);

	Instruction immutableString (std::vector <Var> & vars, Expression expr);
	
	Instruction immutableTuple (std::vector <Var> & vars, Expression expr);

	Instruction immutableMacro (std::vector <Var> & vars, Expression expr);
	
    };

    typedef IFor* For;
    
}
