#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    class Atomic : public IExpression {

	Expression _who;

	Expression _content;

    private :

	friend Expression;

	Atomic ();
	
	Atomic (const lexing::Word & loc, const Expression & who, const Expression & content);

    public :

	static Expression init (const lexing::Word & loc, const Expression & who, const Expression &content);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getWho () const;

	const Expression & getContent () const;

	std::string prettyString () const override;

    protected :

	const std::set <std::string> & computeSubVarNames () override;
	
    };    

}
