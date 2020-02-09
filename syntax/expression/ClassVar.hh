#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct ClassVar
     */
    class ClassVar : public IExpression {	
    private :

	friend Expression;

	ClassVar ();
	
	ClassVar (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;
	
    };    

}
