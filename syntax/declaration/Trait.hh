#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    class Trait : public IDeclaration {
	
	/** the name of the vars */
	std::vector <lexing::Word> _varNames;

	/** The type of the vars */
	std::vector <Expression> _varTypes;

	/** The name of the prototypes */
	std::vector <lexing::Word> _protoNames;

	/** The prototypes declared inside the trait */
	std::vector <Function::Prototype> _prototypes;

	/** The name of the trait */
	lexing::Word _name;
	
    private :

	friend Declaration;

	Trait ();

    public :

	/** 
	 * \brief Create an empty trait
	 */
	static Declaration init ();

	/** 
	 * \brief Create a trait from copy
	 * \param trait the trait to copy
	*/
	static Declaration init (const Trait & trait);

	/**
	 * \brief Create a new trait
	 * \param name the name of the trait 
	 * \param varNames the name of the var declared inside the trait
	 * \param varTypes the type of the var declared
	 * \param protoNames the name of the prototypes
	 * \param protos the prototypes of the methods
	 */
	static Declaration init (const lexing::Word & name, const std::vector <lexing::Word> & varNames, const std::vector <Expression> &varTypes, const std::vector <lexing::Word> & protoNames, const std::vector <Function::Prototype> & protos);

	/**
	 * Mandatory function for proxy polymoprhism system
	 */
	Declaration clone () const override;

	/**
	 * Mandatory function for dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
				
    };

    
}
