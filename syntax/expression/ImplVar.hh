#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct ImplVar
     */
    class ImplVar : public IExpression {

	Expression _right;
	
    private :

	friend Expression;

	ImplVar ();
	
	ImplVar (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token, const Expression & type);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getType () const;

	std::string prettyString () const override;
	
    };    

}
