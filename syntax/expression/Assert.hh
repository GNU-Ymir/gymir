#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Assert
     * Represent a call to a intrinsict function that abort the programm if the test fails
     * \verbatim
     assert := 'assert' '(' expression (',' expression)? ')'
     \endverbatim 
     */
    class Assert : public IExpression {

	/** The location of the assert */
	lexing::Word _location;
	
	/** The test of the assert */
	Expression _test;

	/** The message to display if the test fails */
	Expression _msg;

    private :

	friend Expression;
	
	Assert ();
	
    public :

	/**
	 * \brief Make a copy 
	 * \param asrt the assert to copy
	 */
	static Expression init (const Assert & asrt);

	/**
	 * \brief Create a new assertion 
	 * \param location the location of the assert
	 * \param test the test of the assertion
	 * \param msg the message of the assertion
	 */
	static Expression init (const lexing::Word & location, const Expression & test, const Expression & msg);

	
	Expression clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	bool isOf (const IExpression * type) const override;

    };
    
}
