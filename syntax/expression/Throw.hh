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

	/** The value that is thrown */
	Expression _value;

    private :

	friend Expression;

	Throw ();
	
	Throw (const lexing::Word & loc, const Expression & value);

    public :

	/**
	 * \brief Create a new throw expression
	 * \param location the location of the expression 
	 * \param value the expression that is thrown
	 */
	static Expression init (const lexing::Word & location, const Expression & value);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the value that is thrown
	 */
	const Expression & getValue () const;

	std::string prettyString () const override;
	
    };

}
