#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct List
     * A list, is a sequence of expression surrounded by () or []
     * It can generate array or tuple at semantic time     
     * Do not confuse with Set
     */
    class List : public IExpression {

	/** The location of the expression ('[' or '(' token) */
	lexing::Word _location;

	/** The location of the end of this expression (']' or ')' token) */
	lexing::Word _end;

	/** The parameters of the access */
	std::vector <Expression> _params;
	
    private :

	friend Expression;

	List ();

    public :

	/**
	 * \brief Make a copy of a mult operator
	 * \param op the operator to copy
	 */
	static Expression init (const List & op);

	/**
	 * \brief Create a new Multiple operator
	 * \param location the location of the operation 
	 * \param end the end of the operation
	 * \param element the left operand
	 * \param params the parameters of the operation
	 */
	static Expression init (const lexing::Word & location, const lexing::Word & end, const std::vector <Expression> & params);
	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };

}
