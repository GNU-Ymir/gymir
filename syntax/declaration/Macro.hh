#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Macro
     *  Declaration of a macro
     */
    class Macro : public IDeclaration {
	       
	std::vector <Declaration> _content;

    private :

	friend Declaration;

	Macro ();

	Macro (const lexing::Word & loc, const std::string & comment, const std::vector <Declaration> & content);

    public :

	static Declaration init (const lexing::Word & loc, const std::string & comment, const std::vector <Declaration> & content);

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;
	
	const std::vector <Declaration> & getContent () const;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };

}
