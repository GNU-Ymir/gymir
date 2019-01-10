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

	/** The module to import */
	std::vector <lexing::Word> _module;

	/** The name for rename import (may be empty ()) */
	lexing::Word _as;


    private :

	friend Declaration;

	Import ();

    public :

	/**
	 * \brief Create an empty import
	 */
	static Declaration init ();

	/** 
	 * \brief Create an import from a copy
	 */
	static Declaration init (const Import & imp);

	/**
	 * \brief Create a new import
	 * \param module the module import
	 * \param as the name for renamed import
	 */
	static Declaration init (const std::vector <lexing::Word> & module, const lexing::Word & as);

	/**
	 * \brief Create a clone of the import, mandatory function for proxy polymorphism
	 */
	Declaration clone () const override;

	/**
	 * \brief Mandatory function for proxy dynamic casting
	 */
	bool isOf (const IDeclaration* type) const override;

	/**
	 * \brief Change the module to import
	 * \param name the name of the module
	 */
	void setModule (const std::vector <lexing::Word> & name);

	/**
	 * \brief Change the name of the import
	 * \param name the rename name
	 */
	void setName (const lexing::Word & name);
	
    };
    
}
