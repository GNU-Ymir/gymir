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

	ExpressionWrapper (const lexing::Word & loc, const Expression & content);
	
    public : 

	/**
	 * \brief Init a wrapper for content
	 * \param content the content of the wrapper
	 */
	static Declaration init (const lexing::Word & loc, const Expression & content);

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
