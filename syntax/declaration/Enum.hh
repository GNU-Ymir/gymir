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

	/** The identifier of the enumeration */
	lexing::Word _ident;

	/** The values (var_decls) inside the enumeration declaration */
	std::vector <Expression> _values;

	/** The type of the expression (might be empty) */
	Expression _type;

    private:

	friend Declaration; // Needed for dynamic casting 
	
	Enum ();
	
    public: 

	/**
	 * \brief Create an empty enum
	 */
	static Declaration init ();

	/**
	 * \brief Create a new enumeration
	 * \param ident the name and location of the enumeration
	 * \param values the values inside the enum
	 */
	static Declaration init (const lexing::Word& ident, const std::vector <Expression> & values);


	/**
	 * \brief Create a new enumeration
	 * \param ident the name and location of the enumeration
	 * \param type the type of the enumeration
	 * \param values the values inside the enum
	 */
	static Declaration init (const lexing::Word& ident, const Expression & type, const std::vector <Expression> & values);

	/**
	 * \brief Create a new enum from another one
	 * \param en the enum to copy
	 */
	static Declaration init (const Enum& en);


	/**
	 * \brief Mandatory function used for proxy polymoprhism system
	 */
	Declaration clone () const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;


	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;
	
	/**
	 * \brief set the identifier of the enum
	 * \param ident the new name of the enum
	 */
	void setName (const lexing::Word& ident);

	/**
	 * \return the name and location of the enum
	 */
	const lexing::Word & getName () const;
		
	/**
	 * \brief Add a new attribute to the enum
	 * \param value the value of the attrbibute
	 */
	void addValue (const Expression& value);

	/**
	 * \brief Change the type of the enum
	 * \param expr the new type of the enum
	 */
	void setType (const Expression& expr);

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
