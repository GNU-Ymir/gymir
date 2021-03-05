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


	std::vector <std::string> _field_comments;
	
    private :

	friend Declaration;

	Struct ();

	Struct (const lexing::Word & name, const std::string & comment, const std::vector <lexing::Word> & attrs, const std::vector <Expression> & vars, const std::vector <std::string> & comments);
	
    public :

	/**
	 * \brief Create a new struct
	 * \param name the name of the structure
	 * \param attrs the attributes of the struct
	 * \param decl the declaration of the struct
	 */
	static Declaration init (const lexing::Word & name, const std::string & comment, const std::vector <lexing::Word> & attrs, const std::vector <Expression> & vars, const std::vector <std::string> & comments);


	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the list of custom attributes
	 */
	const std::vector <lexing::Word> & getCustomAttributes () const;

	/**
	 * \return the declaration of the attributes
	 */
	const std::vector <Expression> & getDeclarations () const;

	/**
	 * \return the comments on the fields
	 */
	const std::vector <std::string> & getDeclComments () const;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };

}
