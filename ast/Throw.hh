#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IThrow
     * The syntaxic node representation of an assert instruction 
     * \verbatim
     throw := 'throw' expression ';'
     \endverbatim
     */
    class IThrow : public IInstruction {

	Expression _expr;

    public :

	/**
	 * \param token the location of the instruction
	 * \param test the test of the assertion
	 * \param msg the message to print if the assertion failed
	 * \param isStatic is the assertion at compile time ?
	 */
	IThrow (Word token, Expression expr);
	
	Instruction instruction () override;

	Instruction templateReplace (const std::map <std::string, Expression>&) override;

	std::vector <semantic::Symbol> allInnerDecls () override;

	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual ~IThrow ();

    private :

	Ymir::Tree callPrint (Ymir::Tree);
	
    };

    typedef IThrow* Throw;
    
}
