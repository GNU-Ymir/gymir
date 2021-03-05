#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    /**
     * \struct Import
     * Class defining an importation 
     * Define only one import multiple import are splitted 
     * \verbatim
     import := 'import' name (',' name)* ';'
     name := Identifier ('::' Identifier)* ('::' '_')? ('as' Identifier)?
     \endverbatim
     */
    class Import : public IDeclaration {

	lexing::Word _module;
	
    private :

	friend Declaration;

	Import (const lexing::Word & loc, const std::string & comment, const lexing::Word & module);
	
    public :

	/**
	 * \brief Create a new import
	 * \param module the module import
	 * \param as the name for renamed import
	 */
	static Declaration init (const lexing::Word & loc, const std::string & comment, const lexing::Word & module);


	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the module to import
	 */
	const lexing::Word & getModule () const;

	/**
	 * \return the relative path of the module to import
	 */
	std::string getPath () const;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };
    
}
