#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Dollar
     */
    class Dollar : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

    private :

	friend Expression;

	Dollar ();

    public :

	static Expression init (const Dollar & blk);

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
