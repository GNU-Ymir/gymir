#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>


namespace syntax {

    /**
     * \struct Module
     * Declaration of a module (global or local)
     * \verbatim
     module := 'mod' name ';' 
               | 'mod' Identifier (template)? '{' declaration* '}'

     name := Identifier ('.' Identifier)*
     \endverbatim
     */
    class Module : public IDeclaration {

	static const std::string KEYWORD;
	
	/** The identifier of the module */
	lexing::Word _ident;

	/** All the declaration inside the module */
	std::vector <Declaration> _decls;
	
	/** Is this a global module (entire file) */
	bool _isGlobal;

	/** The function called at static init of the module (may be empty ()); */	 
	Declaration _constructor;

	/** The function called at static destruction of the module (may be empty ()); */
	Declaration _destructor;
	
    private :

	friend Declaration; // Needed for dynamic casting

	Module ();
	
    public:

	/**
	 * \brief Create an empty module
	 */
	static Declaration init ();
	
	/**
	 * \brief Create a new module 
	 * \param ident the location and name of the module
	 * \param decls the list of declaration inside the module
	 */
	static Declaration init (const lexing::Word &ident, const std::vector <Declaration> & decls);

	/**
	 * \brief Create a new module from another one
	 * \param mod the module to copy
	 */
	static Declaration init (const Module & mod);	
	
	/**
	 * \brief Mandatory function used for proxy polymoprhism system
	 */
	Declaration clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
	
	/**
	 * \brief Set the identifier of the module
	 * \param ident the new name of the module
	 */
	void setIdent (const lexing::Word &ident);

	/**
	 * \brief Add a new declaration inside the module
	 * \param decl the declaration to add
	 */
	void addDeclaration (const Declaration & decl);

	/**
	 * \param isGlobal is this module a global one ?
	 */
	void isGlobal (bool isGlobal);
	
    };    

}
