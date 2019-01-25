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

	/** The value of the break */
	Expression _value;

    private :

	friend Expression;

	Break ();
	
	Break (const lexing::Word & loc);

    public :

	/**
	 * \brief Initialize a new break expression 
	 * \param location the location of the break expr
	 * \param value the value of the break
	 */
	static Expression init (const lexing::Word & location, const Expression & value);
	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;	

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

    };

}
