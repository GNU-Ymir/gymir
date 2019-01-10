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

	/** The name of the attributes */
	std::vector <lexing::Word> _varNames;

	/** The types of the attributes */
	std::vector <Expression> _varTypes;

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
	 * \param names the names of the attributes
	 * \param types the types of the attributes
	 */
	static Declaration init (const lexing::Word & name, const std::vector <lexing::Word> & names, const std::vector <Expression> & types);


	/**
	 * Mandatory function for proxy polymoprhism system
	 */
	Declaration clone () const override;

	/**
	 * Mandatory function for dynamic cast
	 */
	bool isOf (const IDeclaration * type) const override;

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
	 * Add a new attribute to the struct
	 * \param name the name of the attribute
	 * \param type the type of the attribute
	 */
	void addAttribute (const lexing::Word & name, const Expression & type);
	
    };

}
