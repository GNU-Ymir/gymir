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
    class ExpressionWrapper : public IDeclaration {

	/** The content of the wrapper */
	Expression _content;

    private :

	friend Declaration; // Needed for dynamic casting 
	
	ExpressionWrapper ();
	
    public : 

	/**
	 * \brief Create an empty wrapper
	 */
	static Declaration init ();

	/**
	 * \brief Copy another expression wrapper
	 */
	static Declaration init (const ExpressionWrapper & gl);

	/**
	 * \brief Init a wrapper for content
	 * \param content the content of the wrapper
	 */
	static Declaration init (const Expression & content);
	
	/**
	 * \brief Mandatory function used by proxy polymoprhism system
	 */
	Declaration clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
	
	/**
	 * \return the content of the wrapper
	 */
	const Expression & getContent () const;
	
    };
    
}
