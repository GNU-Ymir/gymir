#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {

    class IIf;
    typedef IIf* If;

    /**
     * \struct IIf
     * The syntaxic node representation of a if statement
     * \verbatim
     if := 'if' expression block ('else' if | 'else' block)?
     \endverbatim
     */
    class IIf : public IExpression {

	/** The expression to test, may be null in the case of only else statment */
	Expression _test;

	/** The body of the if */
	Block _block;

	/** The else instruction, may be null */
	If _else;

	/** The type of the expression, used to cast to bool */
	semantic::InfoType _info;
	
    public:

	/**
	 * \param word the location of the statement
	 * \param test the test expression
	 * \param block the body
	 * \param isStatic is the condition to be test at compile time ?
	 */
	IIf (Word word, Expression test, Block block, bool isStatic = false);

	/**
	 * \param word the location of the statement
	 * \param test the test expression
	 * \param block the body
	 * \param else_ the else part
	 * \param isStatic is the condition to be test at compile time ?
	 */
	IIf (Word word, Expression test, Block block, If else_, bool isStatic = false);

	Instruction instruction () override;	

	Expression expression () override;
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;
	
    };
    
}
