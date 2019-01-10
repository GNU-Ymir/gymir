#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Is
     */
    class Is : public IExpression {

	lexing::Word _location;

	/** The element to test */
	Expression _element;

	/** The parameters of the access */
	std::vector <Expression> _params;
	
    private :

	friend Expression;

	Is ();

    public :

	/**
	 * \brief Make a copy of a mult operator
	 * \param op the operator to copy
	 */
	static Expression init (const Is & op);

	/**
	 * \brief Create a new Multiple operator
	 * \param location the location of the operation 
	 * \param end the end of the operation
	 * \param element the left operand
	 * \param params the parameters of the operation
	 */
	static Expression init (const lexing::Word & location, const Expression & element, const std::vector <Expression> & params);
	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };

}
