#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>

namespace syntax {

    /**
     * \struct DecoratedExpression
     */
    class DecoratedExpression : public IExpression {

	/** The list of decorator */
	std::vector <DecoratorWord> _decos;

	Expression _content;
	
    private :

	friend Expression;

	DecoratedExpression ();
	
	DecoratedExpression (const lexing::Word & loc, const std::vector <DecoratorWord> & decos, const Expression & content);

    public :

	static Expression init (const lexing::Word & token, const std::vector <DecoratorWord> & decos, const Expression & content);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the content of the expression
	 */
	const Expression & getContent () const;
	
	/**
	 * \return is this expression decorated with deco ?
	 */
	bool hasDecorator (Decorator deco) const;

	/**
	 */
	const DecoratorWord & getDecorator (Decorator deco) const;

	/**
	 * \return the list of decorators;
	 */
	const std::vector <DecoratorWord> & getDecorators () const;
	
	std::string prettyDecorators () const;
	
	std::string prettyString () const override;
	
    };    

}
