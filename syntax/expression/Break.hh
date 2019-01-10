#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Break
     * Represent a break expression
     * \verbatim
     break := 'break' (expression)? ';'
     \endverbatim
     */
    class Break : public IExpression {

	/** The location of the break */
	lexing::Word _location;

	/** The value of the break */
	Expression _value;

    private :

	friend Expression;

	Break ();

    public :

	/**
	 * \brief Make a copy
	 * \param brk the break to copy
	 */
	static Expression init (const Break & brk);

	/**
	 * \brief Initialize a new break expression 
	 * \param location the location of the break expr
	 * \param value the value of the break
	 */
	static Expression init (const lexing::Word & location, const Expression & value);
	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;	

    };

}
