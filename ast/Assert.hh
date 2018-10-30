#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IAssert
     * The syntaxic node representation of an assert instruction 
     * \verbatim
     assert := ('cte')? 'assert' '(' expression (',' expression)? ')'
     \endverbatim
     */
    class IAssert : public IInstruction {

	Expression _expr;

	Expression _msg;

    public :

	/**
	 * \param token the location of the instruction
	 * \param test the test of the assertion
	 * \param msg the message to print if the assertion failed
	 * \param isStatic is the assertion at compile time ?
	 */
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
