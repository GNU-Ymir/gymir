#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    class Use : public IDeclaration {
    	
	/** the name of the vars */
	Expression _module;

	/** The name of the trait */
	lexing::Word _location;
	
    private :

	friend Declaration;

	Use ();

    public :

	/** 
	 * \brief Create an empty use
	 */
	static Declaration init ();

	/** 
	 * \brief Create a use from copy
	 * \param use the use to copy
	*/
	static Declaration init (const Use & use);

	/**
	 * \brief Create a new trait
	 * \param location the location of the use
	 * \param module the module to use
	 */
	static Declaration init (const lexing::Word & location, const Expression & module);

	/**
	 * Mandatory function for proxy polymoprhism system
	 */
	Declaration clone () const override;

	/**
	 * Mandatory function for dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
				
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const lexing::Word & getLocation () const;

	const Expression & getModule () const;
	
    };
    


}
