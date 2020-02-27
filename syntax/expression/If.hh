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

	/** The test of the loop */
	Expression _test;

	/** The content of the loop */
	Expression _content;

	/** The else part */
	Expression _else;
	
    private :

	friend Expression;

	If ();
	
	If (const lexing::Word & loc, const Expression & test, const Expression & content, const Expression & elsePart);

    public :
	 
	/**
	 * \brief Create a new if 
	 * \param location the location of the loop
	 * \param test the test of the expression
	 * \param content the content of the loop (executed at each loop)
	 * \param else the else part
	 */
	static Expression init (const lexing::Word & location, const Expression & test, const Expression & content, const Expression & elsePart);       

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the test of the if 
	 */
	const Expression & getTest () const;

	/**
	 * \return the content of the if expression 
	 */
	const Expression & getContent () const;

	/**
	 * \return the else part of the if
	 */
	const Expression & getElsePart () const;
	
    };
    
}
