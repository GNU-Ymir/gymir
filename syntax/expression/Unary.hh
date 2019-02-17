#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Unary
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Unary : public IExpression {

	/** The size of the allocation */
	Expression _content;

    private :

	friend Expression;

	Unary ();
	
	Unary (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & op, const Expression &content);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the operator of the expression
	 */
	const lexing::Word & getOperator () const;
	
	/**
	 * \return the content of the Unary
	 */
	const Expression & getContent () const;
	
    };    

}
