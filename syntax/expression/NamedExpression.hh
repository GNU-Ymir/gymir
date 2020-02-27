#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>

namespace syntax {

    /**
     * \struct NamedExpression
     */
    class NamedExpression : public IExpression {
	
	Expression _content;
	
    private :

	friend Expression;

	NamedExpression ();
	
	NamedExpression (const lexing::Word & loc, const Expression & content);

    public :

	static Expression init (const lexing::Word & token, const Expression & content);

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the content of the expression
	 */
	const Expression & getContent () const;

    };    

}
