#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    /**
     * \struct Enum
     * Declaration of a enumeration 
     * \verbatim
     enum := 'enum' (':' type)? ('|' Identifiant '=' expression)+ '->' Identifiant ';'
     \endverbatim
     */
    class Enum : public IDeclaration {

	/** The values (var_decls) inside the enumeration declaration */
	std::vector <Expression> _values;

	/** The type of the expression (might be empty) */
	Expression _type;

    private:

	friend Declaration; // Needed for dynamic casting 
	
	Enum ();

	Enum (const lexing::Word& ident, const std::string & comment, const Expression & type, const std::vector <Expression> & values);
	
    public: 

	/**
	 * \brief Create a new enumeration
	 * \param ident the name and location of the enumeration
	 * \param type the type of the enumeration
	 * \param values the values inside the enum
	 */
	static Declaration init (const lexing::Word& ident, const std::string & comment, const Expression & type, const std::vector <Expression> & values);

	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;


	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;	
		
	/**
	 * \return the type of the enumeration
	 */
	const Expression & getType () const;

	/**
	 * \return the different values declared inside the enumeration
	 */
	const std::vector <Expression> & getValues () const;
	
    };
    
}
