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
	lexing::Word _ident;

	/** The value of the variable (may be empty () if !_type.empty ())*/
	Expression _value;

	/** The type of the variable (may be empty () if !_value.empty ())*/
	Expression _type;

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
	 * \param ident the name of the variable
	 * \param type the type of the variable
	 * \param value the value of the variable
	 */
	static Declaration init (const lexing::Word & ident, const Expression & type, const Expression & value);

	/**
	 * \brief Mandatory function used by proxy polymoprhism system
	 */
	Declaration clone () const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
	
	/**
	 * \brief Change the name of the variable
	 * \param name the new name of the variable
	 */
	void setName (const lexing::Word & name);

	/**
	 * \brief Set the value of the global var
	 * \param value the new value
	 */
	void setValue (const Expression & value);


	/**
	 * \brief Set the type of the global
	 * \param type the new type
	 */
	void setType (const Expression & type);
	
    };
    
}
