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
	
	Unary (const lexing::Word & loc, const Expression &content);

    public :

	static Expression init (const lexing::Word & op, const Expression &content);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the operator of the expression
	 */
	const lexing::Word & getOperator () const;
	
	/**
	 * \return the content of the Unary
	 */
	const Expression & getContent () const;

	std::string prettyString () const override;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };    

}
