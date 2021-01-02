#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroMult
     */
    class MacroOr : public IExpression {
	
	Expression _left;

	Expression _right;
	
    private :

	friend Expression;

	MacroOr ();
	
	MacroOr (const lexing::Word & loc, const Expression & left, const Expression & right);

    public :

	static Expression init (const lexing::Word & token, const Expression & left, const Expression & right);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getLeft () const;

	const Expression & getRight () const;
	
	std::string prettyString () const override;
	
    };    

}
