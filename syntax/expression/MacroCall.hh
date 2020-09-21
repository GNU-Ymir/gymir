#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroCall
     */
    class MacroCall : public IExpression {

	lexing::Word _end;
	
	/** The left operand (macro) */
	Expression _left;

	std::string _content;
	
    private :

	friend Expression;

	MacroCall ();
	
	MacroCall (const lexing::Word & loc, const lexing::Word & end, const Expression & left, const std::string & content);

    public :

	static Expression init (const lexing::Word & location, const lexing::Word & end, const Expression & left, const std::string & content);

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const lexing::Word & getEnd () const;
	
	const Expression & getLeft () const;
	
	std::string prettyString () const override;

	const std::string & getContent () const;
	
    };    

}
