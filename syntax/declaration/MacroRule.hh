#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <vector>

namespace syntax {

    /**
     * \struct MacroRule
     *  Declaration of a macrorule
     */
    class MacroRule : public IDeclaration {
	
	Expression _rule;

	lexing::Word _contentLoc;
	
	std::string _content;	
	
	std::vector<Expression> _skips;
	
    private :

	friend Declaration;

	MacroRule ();

	MacroRule (const lexing::Word & loc, const lexing::Word & contentLoc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector <Expression> & skips);

    public :

	static Declaration init (const lexing::Word & loc, const lexing::Word & contentLoc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector <Expression> & skips);

	static Declaration init (const MacroRule & rule);

	bool isOf (const IDeclaration * type) const override;       	

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;

	const std::string & getContent () const;

	const lexing::Word & getContentLoc () const;
	
	const Expression & getRule () const;	
	
	const std::vector<Expression> & getSkips () const;
	
    };

}
