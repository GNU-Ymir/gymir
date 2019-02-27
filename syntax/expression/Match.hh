#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Match
     */
    class Match : public IExpression {

    private :

	friend Expression;

	Match ();
	
	Match (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
