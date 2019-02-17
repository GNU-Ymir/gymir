#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct While
     * Representation of a while loop
     * \verbatim
     while := 'while' expression expression
     \endverbatim
     */
    class While : public IExpression {

	/** The test of the loop (could be empty iif it is a 'loop' expression) */
	Expression _test;

	/** The content of the loop */
	Expression _content;

	/** set to true if it is a do while expression */
	bool _isDo;
	
    private :

	friend Expression;

	While ();
	
	While (const lexing::Word & loc);

    public :

	/**
	 * \brief Create a new while loop
	 * \param location the location of the loop
	 * \param test the test of the expression
	 * \param content the content of the loop (executed at each loop)
	 */
	static Expression init (const lexing::Word & location, const Expression & test, const Expression & content, bool isDo = false);

	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the test of the loop
	 */
	const Expression & getTest () const;

	/**
	 * \return the content of the loop
	 */
	const Expression & getContent () const;

	/**
	 * \return is this expression a do {} while () ?
	 */
	bool isDo () const;

    };
    
}
