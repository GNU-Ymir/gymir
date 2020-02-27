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

	/** The value of the alias */
	Expression _value;

    private :

	friend Declaration; // Needed for dynamic casting 

	Alias ();
	
	Alias (const lexing::Word & ident, const Expression & value);
	
    public :

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


	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;	
	
	/**
	 * \return the value of the alias
	 */
	const Expression & getValue () const ;
	
    };    

}
