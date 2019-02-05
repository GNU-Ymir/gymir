#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>

namespace syntax {

    /**
     * \struct Var
     */
    class Var : public IExpression {
    private :

	friend Expression;

	Var ();
	
	Var (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the name of the var (same as getLocation)
	 */
	const lexing::Word & getName () const;
	

    };    

}
