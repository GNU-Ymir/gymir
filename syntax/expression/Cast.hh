#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Cast
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Cast : public IExpression {

	/** The location of the allocation */
	lexing::Word _op;

	/** The left operand, can be a type or a value */
	Expression _type;

	/** The size of the allocation */
	Expression _content;

    private :

	friend Expression;

	Cast ();

    public :

	static Expression init (const Cast & alloc);

	static Expression init (const lexing::Word & op, const Expression & type, const Expression &content);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
