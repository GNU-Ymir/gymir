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
	 * - function (methods)
	 * - vardecl (inside a wrapper, as VarDecl is an expression)
	 * constructor are function with keyword name (self), that return void
	 */
	std::vector <Declaration> _inner;
	
	
    private :

	friend Declaration;
	
	Trait ();

	Trait (const lexing::Word & name, const std::string & comment, const std::vector <Declaration> & inner);

    public :

	/**
	 * \brief Create a new trait
	 * \param name the name of the trait 
	 * \param inner the declaration inside the trait
	 */
	static Declaration init (const lexing::Word & name, const std::string & comment, const std::vector <Declaration> & inner);
		

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	const std::vector <Declaration> & getDeclarations () const;

    };
    
}
