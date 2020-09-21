#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroConstructor
     *  Declaration of a macrorule
     */
    class MacroConstructor : public IDeclaration {
	
	Expression _rule;
	
	Expression _type;

	Expression _content;

    private :

	friend Declaration;

	MacroConstructor ();

	MacroConstructor (const lexing::Word & loc, const Expression & type, const Expression & rule, const Expression & content);

    public :

	static Declaration init (const lexing::Word & loc, const Expression & type, const Expression & rule, const Expression & content);

	static Declaration init (const MacroConstructor & contr);

	bool isOf (const IDeclaration * type) const override;       	

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;

	const Expression & getRule () const;

	const Expression & getType () const;

	const Expression & getContent () const;
	
    };

}
