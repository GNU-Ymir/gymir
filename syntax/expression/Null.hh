#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Null
     */
    class Null : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

    private :

	friend Expression;

	Null ();

    public :

	static Expression init (const Null & blk);

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
