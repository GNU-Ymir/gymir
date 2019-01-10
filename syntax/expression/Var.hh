#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Var
     */
    class Var : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

    private :

	friend Expression;

	Var ();

    public :

	static Expression init (const Var & blk);

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
