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
	
	/** The test of the assert */
	Expression _test;

	/** The message to display if the test fails */
	Expression _msg;

    private :

	friend Expression;

	Assert ();
	
	Assert (const lexing::Word & loc, const Expression & test, const Expression & msg);
	
    public :

	/**
	 * \brief Create a new assertion 
	 * \param location the location of the assert
	 * \param test the test of the assertion
	 * \param msg the message of the assertion
	 */
	static Expression init (const lexing::Word & location, const Expression & test, const Expression & msg);
	
	const Expression & getTest () const;

	const Expression & getMsg () const;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;	

	std::string prettyString () const override;
	
    };
    
}
