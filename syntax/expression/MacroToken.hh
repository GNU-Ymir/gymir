#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroMult
     */
    class MacroToken : public IExpression {
	
	Expression _content;
	
    private :

	friend Expression;

	MacroToken ();
	
	MacroToken (const lexing::Word & loc, const Expression & content);

    public :

	static Expression init (const lexing::Word & token, const Expression & content);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

	const Expression & getContent () const;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };    

}
