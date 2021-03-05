#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {


    class Try : public IExpression {
    private :

	Expression _inner;

    private :

	friend Expression;

	Try ();

	Try (const lexing::Word & loc, const Expression & inner);

    public : 

	static Expression init (const lexing::Word & token, const Expression & inner);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;	

	const syntax::Expression & getContent () const;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };
    
}
