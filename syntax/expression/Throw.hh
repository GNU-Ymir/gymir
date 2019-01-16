#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Throw
     * A throw expression, is the action of raising exception
     * \verbatim
     throw := 'throw' expression ';'
     \endverbatim
     */
    class Throw : public IExpression {

	/** The location of the expression */
	lexing::Word _location;

	/** The value that is thrown */
	Expression _value;

    private :

	friend Expression;

	Throw ();

    public :

	/**
	 * \brief Make a copy of the throw
	 * \param thr the content of the throw
	 */
	static Expression init (const Throw & thr);

	/**
	 * \brief Create a new throw expression
	 * \param location the location of the expression 
	 * \param value the expression that is thrown
	 */
	static Expression init (const lexing::Word & location, const Expression & value);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };

}
