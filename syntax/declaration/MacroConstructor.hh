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

	lexing::Word _contentLoc;
	
	std::string _content;

	std::vector <Expression> _skips;
	
    private :

	friend Declaration;

	MacroConstructor ();

	MacroConstructor (const lexing::Word & loc, const lexing::Word & contentLoc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector <Expression> & skips);

    public :

	static Declaration init (const lexing::Word & loc, const lexing::Word & contentLoc, const std::string & comment, const Expression & rule, const std::string & content, const std::vector <Expression> & skips);

	static Declaration init (const MacroConstructor & contr);

	bool isOf (const IDeclaration * type) const override;       	

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;

	const Expression & getRule () const;
	
	const std::string & getContent () const;

	const lexing::Word & getContentLoc () const;
	
	const std::vector <Expression> & getSkips () const;
	
    };

}
