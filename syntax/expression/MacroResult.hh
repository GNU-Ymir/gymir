#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroMult
     */
    class MacroResult : public IExpression {
	
	std::string _content;
	
    private :

	friend Expression;

	MacroResult ();
	
	MacroResult (const lexing::Word & loc, const std::string & content);

    public :

	static Expression init (const lexing::Word & token, const std::string & content);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

	const std::string & getContent () const;
	
    };    

}
