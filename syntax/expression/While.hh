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

	/** The location of the loop */
	lexing::Word _location;

	/** The test of the loop */
	Expression _test;

	/** The content of the loop */
	Expression _content;

    private :

	friend Expression;

	While ();

    public :

	/**
	 * \brief Make a copy of a while loop
	 * \param wh the loop to copy
	 */
	static Expression init (const While & wh);

	/**
	 * \brief Create a new while loop
	 * \param location the location of the loop
	 * \param test the test of the expression
	 * \param content the content of the loop (executed at each loop)
	 */
	static Expression init (const lexing::Word & location, const Expression & test, const Expression & content);

	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };
    
}
