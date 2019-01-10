#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    class FuncPtr : public IExpression {

	/** The location of the expression ('[' or '(' token) */
	lexing::Word _location;

	/** The parameters of the function */
	std::vector <Expression> _params;

	/** The return type */
	Expression _ret;
	
    private :

	friend Expression;

	FuncPtr ();

    public :

	/**
	 * \brief Make a copy of a mult operator
	 * \param op the operator to copy
	 */
	static Expression init (const FuncPtr & op);

	/**
	 * \brief Create a new Multiple operator
	 * \param location the location of the operation 
	 * \param end the end of the operation
	 * \param element the left operand
	 * \param params the parameters of the operation
	 */
	static Expression init (const lexing::Word & location, const Expression & ret, const std::vector <Expression> & params);
	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };

}
