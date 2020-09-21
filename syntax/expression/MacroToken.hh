#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroMult
     */
    class MacroToken : public IExpression {
	
	std::string _content;
	
    private :

	friend Expression;

	MacroToken ();
	
	MacroToken (const lexing::Word & loc, const std::string & content);

    public :

	static Expression init (const lexing::Word & token, const std::string & content);

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

	const std::string & getContent () const;
	
    };    

}
