#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroEval
     */
    class MacroEval : public IExpression {

	lexing::Word _end;
		
	Expression _content;
	
    private :

	friend Expression;

	MacroEval ();
	
	MacroEval (const lexing::Word & loc, const lexing::Word & end, const Expression & content);

    public :

	static Expression init (const lexing::Word & token, const lexing::Word & end, const Expression & content);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

	const lexing::Word & getEnd () const;
	
	const Expression & getContent () const;
	
    };    

}
