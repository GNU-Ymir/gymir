#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct VariadicVar
     */
    class VariadicVar : public IExpression {

	/** This variable is a value (if ':') */
	bool _isValue;
	
    private :

	friend Expression;

	VariadicVar ();
	
	VariadicVar (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token, bool isValue);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };    

}
