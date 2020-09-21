#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Macro
     *  Declaration of a macro
     */
    class Macro : public IDeclaration {
	
	std::vector <std::string> _skips;
	
	std::vector <Declaration> _content;

    private :

	friend Declaration;

	Macro ();

	Macro (const lexing::Word & loc, const std::vector<std::string> & skips, const std::vector <Declaration> & content);

    public :

	static Declaration init (const lexing::Word & loc, const std::vector<std::string> & skips, const std::vector <Declaration> & content);

	bool isOf (const IDeclaration * type) const override;       	

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;

	const std::vector <std::string> & getSkips () const;
	
	const std::vector <Declaration> & getContent () const;
	
    };

}
