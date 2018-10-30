#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IBreak
     * Syntaxic node representation of a Break instruction 
     * \verbatim
     break := 'break' (Identifier)? ';'
     \endverbatim
     */
    class IBreak : public IInstruction {
	
	Word _ident;
	ulong _nbBlock;

    public:
	
	/**
	 * \param token the location of the break
	 */
	IBreak (Word token);

	/**
	 * \param token the location of the break
	 * \param ident the identifier of the break
	 */
	IBreak (Word token, Word ident);

	Instruction instruction () override;

	Instruction templateReplace (const std::map <std::string, Expression>&) override;
	
	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Ymir::Tree toGeneric () override;

	static std::string id () {
	    return TYPEID (IBreak);
	}
	
	virtual std::vector <std::string> getIds () {
	    auto ret = IInstruction::getIds ();
	    ret.push_back (TYPEID (IBreak));
	    return ret;
	}
	
	void print (int nb = 0) override;
	
    };

    typedef IBreak* Break;
}
