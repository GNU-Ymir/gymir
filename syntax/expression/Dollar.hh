#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Dollar
     */
    class Dollar : public IExpression {
    private :

	friend Expression;

	Dollar ();
	
	Dollar (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };    

}
