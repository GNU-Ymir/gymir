#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/declaration/Function.hh>

namespace syntax {

    /**
     * \struct Lambda
     */
    class Lambda : public IExpression {

	Function::Prototype _proto;
	
	/** The content of the function */
	Expression _content;

    private :

	friend Expression;

	Lambda ();
	
	Lambda (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & op, const Function::Prototype & proto, const Expression & content);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

    };    

}
