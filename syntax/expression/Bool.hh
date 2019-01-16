#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Bool
     */
    class Bool : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

    private :

	friend Expression;

	Bool ();

    public :

	static Expression init (const Bool & blk);

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };    

}
