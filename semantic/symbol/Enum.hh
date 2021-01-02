#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Enum.hh>
#include <ymir/semantic/Generator.hh>
#include <vector>

namespace semantic {

    class Enum : public ISymbol {
	
	/** The symbol declared in the structure (basically the attributes) */
	std::shared_ptr<Table> _table;

	/** The type of the enum */
	syntax::Expression _type;

	/** The values of the enumerations */
	const std::vector <syntax::Expression> _fields;

	/** Set at validation time, to prevent multiple time validation of the same symbol */
	generator::Generator _gen;

	const std::vector <std::string> _field_comments;
	
    private :

	friend Symbol;

	Enum ();

	Enum (const lexing::Word & name, const std::string & comments, const std::vector <syntax::Expression> & fields, const syntax::Expression & type, const std::vector <std::string> & fieldComments, bool isWeak);

	Enum (const Enum & other);
	
    public :

	static Symbol init (const lexing::Word & name, const std::string & comments, const std::vector<syntax::Expression> & fields, const syntax::Expression & type, const std::vector <std::string> & fieldComments, bool isWeak);

	void insert (const Symbol & sym) override;

	void getTemplates (std::vector<Symbol> & rets) const override;

	void insertTemplate (const Symbol & sym) override;

	void replace (const Symbol & sym) override;	

	void get (const std::string & name, std::vector <Symbol> & rets) const override;

	void getPublic (const std::string & name, std::vector <Symbol> & rets) const override;

	void getLocal (const std::string & name, std::vector <Symbol> & rets) const override;

	void getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const override;

	bool equals (const Symbol & other, bool parent = true) const override;

	std::string formatTree (int padd) const override;

	/**
	 * \Warning might be empty
	 * \return the type of the enum
	 */
	const syntax::Expression & getType () const;
	
	/**
	 * \brief The List of fields stores in the syntaxic node, because the type and value of the fields cannot be known before validation time
	 * \brief All the types must be known before trying to type the variables
	 */
	const std::vector<syntax::Expression> & getFields () const;

	/**
	 * \return the comments about the fields
	 */
	const std::vector<std::string> & getFieldComments () const;
	
	/**
	 * \brief This information is set at validation time
	 * \return the generator (enum prototype)
	 */
	const generator::Generator & getGenerator () const;

	/**
	 * \brief This information is set a validation time
	 * \brief set the generator (enum prototype)
	 */
	void setGenerator (const generator::Generator & gen);
	
    };

}


