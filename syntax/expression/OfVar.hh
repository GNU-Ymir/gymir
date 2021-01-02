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
	
	OfVar (const lexing::Word & loc, const Expression & type);

    public :

	static Expression init (const lexing::Word & token, const Expression & type);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getType () const;

	std::string prettyString () const override;
	
    };    

}
