#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct OfVar
     */
    class OfVar : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

	Expression _right;
	
    private :

	friend Expression;

	OfVar ();

    public :

	static Expression init (const OfVar & blk);

	static Expression init (const lexing::Word & token, const Expression & type);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
