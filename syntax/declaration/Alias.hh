#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    /**
     * \struct Alias
     * Declaration of an alias
     * \verbatim
     alias := 'alias' Identifier '=' expression ';'
     \endverbatim
     */
    class Alias : public IDeclaration {

	/** The identifier of the alias*/
	lexing::Word _ident;

	/** The value of the alias */
	Expression _value;

    private :

	friend Declaration; // Needed for dynamic casting 
	
	Alias ();
	
    public :

	/**
	 * \brief Create an empty alias
	 */
	static Declaration init (); 

	/** 
	 * \brief Create a new alias
	 * \param ident the identifier of the alias
	 * \param value the value of the alias
	 */
	static Declaration init (const lexing::Word& ident, const Expression & value);
	
	/**
	 * \brief Create a new Alias from anothe one
	 * \param alias the alias to copy
	 */
	static Declaration init (const Alias & alias);


	/**
	 * \brief Construct a copy, mandatory function for proxy polymoprhism system
	 */
	Declaration clone () const override;

	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;

	/**
	 * \brief Set the identifier of the alias
	 * \param ident the new identifier 
	 */
	void setIdent (const lexing::Word & ident);

	/**
	 * \brief Set the value of the alias
	 * \param value the value of the alias
	 */
	void setValue (const Expression & value);
	
    };    

}
