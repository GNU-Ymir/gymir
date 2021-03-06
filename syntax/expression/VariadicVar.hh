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
	
	VariadicVar (const lexing::Word & loc, bool isValue);

    public :

	static Expression init (const lexing::Word & token, bool isValue);
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;
	
    };    

}
