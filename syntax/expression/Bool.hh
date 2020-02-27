#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Bool
     */
    class Bool : public IExpression {

    private :

	friend Expression;

	Bool ();

	Bool (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };    

}
