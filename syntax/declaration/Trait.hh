#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    class Trait : public IDeclaration {
	
	/** The prototypes declared inside the trait 
	 * It could be : 
	 * - function 
	 * - vardecl (inside a wrapper, as VarDecl is an expression)
	 * constructor and destructor are function with keyword name (self, or self~), that return void
	 */
	std::vector <Declaration> _inner;

	/** The name of the trait */
	lexing::Word _name;

	/** 
	 * True if this is declaration is a mixin 
	 * The difference between trait and mixin is semantic
	 */
	bool _isMixin; 
	
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
	 * \param inner the declaration inside the trait
	 */
	static Declaration init (const lexing::Word & name, const std::vector <Declaration> & inner, bool isMixin);

	/**
	 * Mandatory function for proxy polymoprhism system
	 */
	Declaration clone () const override;
		
	/**
	 * Mandatory function for dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const lexing::Word & getName () const;

	const std::vector <Declaration> & getInners () const;

	bool isMixin () const;
    };

    
}
