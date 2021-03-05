#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Bool
     */
    class Bool : public IExpression {

    private :

	friend Expression;

	Bool ();

	Bool (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

    protected :

	const std::set <std::string> & computeSubVarNames () override;
	
    };    

}
