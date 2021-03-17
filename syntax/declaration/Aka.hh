#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    /**
     * \struct Aka
     * Declaration of an alias
     * \verbatim
     alias := 'alias' Identifier '=' expression ';'
     \endverbatim
     */
    class Aka : public IDeclaration {

	/** The value of the alias */
	Expression _value;

    private :

	friend Declaration; // Needed for dynamic casting 

	Aka ();
	
	Aka (const lexing::Word & ident, const std::string & comment, const Expression & value);
	
    public :

	/** 
	 * \brief Create a new alias
	 * \param ident the identifier of the alias
	 * \param value the value of the alias
	 */
	static Declaration init (const lexing::Word& ident, const std::string & comment, const Expression & value);
	
	/**
	 * \brief Create a new Aka from anothe one
	 * \param alias the alias to copy
	 */
	static Declaration init (const Aka & alias);


	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;
	
	
	/**
	 * \return the value of the alias
	 */
	const Expression & getValue () const ;
	
    };    

}
