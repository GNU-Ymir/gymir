#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroMult
     */
    class MacroVar : public IExpression {
	
	Expression _content;
	
    private :

	friend Expression;

	MacroVar ();
	
	MacroVar (const lexing::Word & loc, const Expression & value);

    public :

	static Expression init (const lexing::Word & token, const Expression & value);

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

	const Expression & getContent () const;
	
    };    

}
