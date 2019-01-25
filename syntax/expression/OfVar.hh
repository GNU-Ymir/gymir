#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct OfVar
     */
    class OfVar : public IExpression {

	Expression _right;
	
    private :

	friend Expression;

	OfVar ();
	
	OfVar (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token, const Expression & type);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };    

}
