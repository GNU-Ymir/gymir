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

	/** The type of the variable (may be empty () if !_value.empty ())*/
	Expression _decl;

    private :

	friend Declaration; // Needed for dynamic casting 
	
	Global ();

	Global (const lexing::Word & loc, const std::string & comment, const Expression & decl);
	
    public : 

	/**
	 * \brief Init a new global var
	 * \param location the location of the declaration 
	 * \param decl the var_decl
	 */
	static Declaration init (const lexing::Word & location, const std::string & comment, const Expression & decl);

	/**
	 * \brief Return the content var declaration
	 */
	const Expression & getContent () const;

	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };
    
}
