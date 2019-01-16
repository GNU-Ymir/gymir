#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    class Mixin : public IDeclaration {
    	
	/** the name of the vars */
	Expression _mixin;

	/** The name of the trait */
	lexing::Word _location;
	
    private :

	friend Declaration;

	Mixin ();

    public :

	/** 
	 * \brief Create an empty use
	 */
	static Declaration init ();

	/** 
	 * \brief Create a use from copy
	 * \param mixin the mixin to copy
	*/
	static Declaration init (const Mixin & mixin);

	/**
	 * \brief Create a new trait
	 * \param location the location of the use
	 * \param mixin the mixin to use
	 */
	static Declaration init (const lexing::Word & location, const Expression & mixin);

	/**
	 * Mandatory function for proxy polymoprhism system
	 */
	Declaration clone () const override;

	/**
	 * Mandatory function for dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };
    


}
