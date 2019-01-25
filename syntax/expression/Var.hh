#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>

namespace syntax {

    /**
     * \struct Var
     */
    class Var : public IExpression {

	/** The decorator of the var if any (ref, const ...) */
	std::vector <Decorator> _decos;
	
    private :

	friend Expression;

	Var ();
	
	Var (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);

	static Expression init (const lexing::Word & token, const std::vector <Decorator> & decos);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const lexing::Word & getName () const;
	
    };    

}
