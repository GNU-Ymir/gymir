#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Match
     */
    class Match : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

    private :

	friend Expression;

	Match ();

    public :

	static Expression init (const Match & blk);

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
