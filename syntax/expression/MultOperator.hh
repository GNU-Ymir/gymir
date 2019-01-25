#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MultOperator
     * There are three different mult operator 
     * Par, Access and Template
     */
    class MultOperator : public IExpression {

	/** The location of the end of this expression (']' or ')' token) */
	lexing::Word _end;

	/** The element that is accessed */
	Expression _element;

	/** The parameters of the access */
	std::vector <Expression> _params;
	
    private :

	friend Expression;

	MultOperator ();

	MultOperator (const lexing::Word & loc);

    public :

	/**
	 * \brief Create a new Multiple operator
	 * \param location the location of the operation 
	 * \param end the end of the operation
	 * \param element the left operand
	 * \param params the parameters of the operation
	 */
	static Expression init (const lexing::Word & location, const lexing::Word & end, const Expression & element, const std::vector <Expression> & params);
	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

    };

}
