#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>


namespace syntax {

    /**
     * \struct Struct
     * A structure representation
     * \verbatim 
     struct := 'struct' attributes templates? ('|' var_decl)? '->' Identifier ';'
     var_decl := Identifier ':' type
     \endverbatim 
     */
    class Struct : public IDeclaration {

	/** The declaration of the attributes */
	std::vector <Expression> _decls;

	/** The CAs (custom attributes) of the struct */
	std::vector <lexing::Word> _cas;

	lexing::Word _name;
	
    private :

	friend Declaration;

	Struct ();

    public :

	/**
	 * \brief Create an empty structure
	 */
	static Declaration init ();

	/**
	 * \brief Create an new struct by copying 
	 * \param str the struct to copy
	 */
	static Declaration init (const Struct & str);

	/**
	 * \brief Create a new struct
	 * \param name the name of the structure
	 * \param attrs the attributes of the struct
	 * \param decl the declaration of the struct
	 */
	static Declaration init (const lexing::Word & name, const std::vector <lexing::Word> & attrs, const std::vector <Expression> & vars);

	/**
	 * Mandatory function for proxy polymoprhism system
	 */
	Declaration clone () const override;

	/**
	 * Mandatory function for dynamic cast
	 */
	bool isOf (const IDeclaration * type) const override;


	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	/**
	 * \brief add a custom attribute to the struct
	 * \param ca the custom attr
	 */
	void addCustomAttribute (const lexing::Word & ca);
	
	/**
	 * Change the name of the struct
	 */
	void setName (const lexing::Word & name);

	/**
	 * \return the name and location of the declaration
	 */
	const lexing::Word & getName () const ;

	/**
	 * \return the list of custom attributes
	 */
	const std::vector <lexing::Word> & getCustomAttributes () const;

	/**
	 * \return the declaration of the attributes
	 */
	const std::vector <Expression> & getDeclarations () const;
    };

}
