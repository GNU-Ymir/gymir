#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroCall
     */
    class MacroCall : public IExpression {

	/** The left operand (macro) */
	Expression _left;

	/** The content tokens */
	std::vector <lexing::Word> _content;	
	
    private :

	friend Expression;

	MacroCall ();
	
	MacroCall (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & location, const Expression & left, const std::vector <lexing::Word> & content);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
