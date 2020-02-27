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
	
	/** All the declaration inside the module */
	std::vector <Declaration> _decls;
	
	/** Is this a global module (entire file) */
	bool _isGlobal;

    private :

	friend Declaration; // Needed for dynamic casting

	Module ();

	Module (const lexing::Word & loc, const std::vector <Declaration> & content, bool isGlobal);
	
    public:
		
	/**
	 * \brief Create a new module 
	 * \param ident the location and name of the module
	 * \param decls the list of declaration inside the module
	 */
	static Declaration init (const lexing::Word &ident, const std::vector <Declaration> & decls, bool isGlobal);

	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
	
	/**
	 * \return is this module global ?
	 */
	bool isGlobal () const;

	/**
	 * \return the list of declaration of the module
	 */
	const std::vector <Declaration> & getDeclarations () const;
	
    };    

}
