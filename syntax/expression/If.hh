#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct If
     * Representation of a while loop
     * \verbatim
     if := 'if' expression expression ('else' if | 'else' expression)?
     \endverbatim
     */
    class If : public IExpression {

	/** The location of the loop */
	lexing::Word _location;

	/** The test of the loop */
	Expression _test;

	/** The content of the loop */
	Expression _content;

	/** The else part */
	Expression _else;
	
    private :

	friend Expression;

	If ();

    public :
	 
	static Expression init (const If & wh);

	/**
	 * \brief Create a new if 
	 * \param location the location of the loop
	 * \param test the test of the expression
	 * \param content the content of the loop (executed at each loop)
	 * \param else the else part
	 */
	static Expression init (const lexing::Word & location, const Expression & test, const Expression & content, const Expression & elsePart);

	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };
    
}
