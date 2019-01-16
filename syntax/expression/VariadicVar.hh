#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct VariadicVar
     */
    class VariadicVar : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

	/** This variable is a value (if ':') */
	bool _isValue;
	
    private :

	friend Expression;

	VariadicVar ();

    public :

	static Expression init (const VariadicVar & blk);

	static Expression init (const lexing::Word & token, bool isValue);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

    };    

}
