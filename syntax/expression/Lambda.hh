#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/declaration/Function.hh>

namespace syntax {

    /**
     * \struct Lambda
     */
    class Lambda : public IExpression {

	/** The location of the allocation */
	lexing::Word _op;

	Function::Prototype _proto;
	
	/** The content of the function */
	Expression _content;

    private :

	friend Expression;

	Lambda ();

    public :

	static Expression init (const Lambda & alloc);

	static Expression init (const lexing::Word & op, const Function::Prototype & proto, const Expression & content);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
