#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    /**
     * \struct IEnum
     * The syntaxic node representation of an enumeration
     * \verbatim
     enum := 'enum' (':' type)? ('|' Identifiant ':' expression)+ '->' Identifiant ';'
     \endverbatim
     */
    class IEnum : public IDeclaration {

	/**
	 * The identifier of this enum
	 */
	Word ident;

	/** The type of this enumeration (may be null) */
	Expression _type;

	/**
	 * The different comment related to the values
	 */
	std::vector <std::string> _docs;

	/**
	 * The name of the attributes
	 */
	std::vector <Word> _names;
	
	/**
	 * THe values of the attributes 
	 */
	std::vector <Expression> _values;

	/**
	 * The type information of this enum
	 */
	semantic::Symbol _info;
	
    public:

	/**
	 * \param ident the location (and identifier) of the enum
	 * \param the documentation related to the declaration 
	 * \param type the type of this declaration (could be null)
	 * \param docsName the comments related to the attributes
	 * \param names the names of the attributes
	 * \param values the values of the attributes
	 */
	IEnum (Word ident, const std::string& docs, Expression type, std::vector <std::string> docsName, std::vector <Word> names, std::vector <Expression> values);

	Ymir::json generateDocs () override;

	void declare () override;
	
	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module mod) override;
	
	void print (int nb = 0) override;

	virtual ~IEnum ();
	
    };

    typedef IEnum* Enum;
    
}
