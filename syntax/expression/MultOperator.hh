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

	bool _canbedotCall = true;
	
    private :

	friend Expression;

	MultOperator (const lexing::Word & loc, const lexing::Word & end, const Expression & element, const std::vector <Expression> & params, bool canbedotcall = true);

    public :

	/**
	 * \brief Create a new Multiple operator
	 * \param location the location of the operation 
	 * \param end the end of the operation
	 * \param element the left operand
	 * \param params the parameters of the operation
	 */
	static Expression init (const lexing::Word & location, const lexing::Word & end, const Expression & element, const std::vector <Expression> & params, bool canbedotcall = true);       
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the location of the closing token of the expression
	 */
	const lexing::Word & getEnd () const;

	/**
	 * \return the left operand of the expression
	 */
	const Expression & getLeft () const;

	/**
	 * \return the right operands of the expression (between the operator tokens)
	 */
	const std::vector <Expression> & getRights () const;

	/**
	 * \brief Does this expression can be used for a dotcall
	 * \brief This param is set at validation for intrinsics validations
	 */
	bool canBeDotCall () const;

	std::string prettyString () const override;
	
    };

}
