#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroRule
     *  Declaration of a macrorule
     */
    class MacroRule : public IDeclaration {
	
	Expression _rule;
	
	Expression _type;

	Expression _content;

    private :

	friend Declaration;

	MacroRule ();

	MacroRule (const lexing::Word & loc, const Expression & rule, const Expression & type, const Expression & content);

    public :

	static Declaration init (const lexing::Word & loc, const Expression & rule, const Expression & type, const Expression & content);

	static Declaration init (const MacroRule & rule);

	bool isOf (const IDeclaration * type) const override;       	

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;

	const Expression & getContent () const;

	const Expression & getRule () const;
	
    };

}
