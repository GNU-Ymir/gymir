#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroCall
     */
    class MacroCall : public IExpression {

	/** The location */
	lexing::Word _location;

	/** The left operand (macro) */
	Expression _left;

	/** The content tokens */
	std::vector <lexing::Word> _content;	
	
    private :

	friend Expression;

	MacroCall ();

    public :

	static Expression init (const MacroCall & blk);

	static Expression init (const lexing::Word & location, const Expression & left, const std::vector <lexing::Word> & content);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
