#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>

namespace syntax {

    /**
     * \struct Global
     * Declaration of a global variable
     * \verbatim
     global := 'static' Identifier (':' type | '=' expression) ';' 
     \endverbatim
     */
    class Global : public IDeclaration {

	/** The name of the variable */
	lexing::Word _location;

	/** The type of the variable (may be empty () if !_value.empty ())*/
	Expression _decl;

    private :

	friend Declaration; // Needed for dynamic casting 
	
	Global ();
	
    public : 

	/**
	 * \brief Create an empty global var
	 */
	static Declaration init ();

	/**
	 * \brief Copy another global var
	 */
	static Declaration init (const Global & gl);

	/**
	 * \brief Init a new global var
	 * \param location the location of the declaration 
	 * \param decl the var_decl
	 */
	static Declaration init (const lexing::Word & location, const Expression & decl);

	/**
	 * \brief Mandatory function used by proxy polymoprhism system
	 */
	Declaration clone () const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;

	/**
	 * \brief Return the content var declaration
	 */
	const Expression & getContent () const;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };
    
}
