#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    class Cast : public IExpression {

	Expression _type;

	Expression _content;

    private :

	friend Expression;

	Cast ();
	
	Cast (const lexing::Word & loc, const Expression & type, const Expression & content);

    public :

	static Expression init (const lexing::Word & op, const Expression & type, const Expression &content);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getType () const;

	const Expression & getContent () const;

	std::string prettyString () const override;

    protected :

	const std::set <std::string> & computeSubVarNames () override;
	
    };    

}
