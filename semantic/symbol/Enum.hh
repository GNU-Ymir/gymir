#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Enum.hh>
#include <ymir/semantic/Generator.hh>
#include <vector>

namespace semantic {

    class Enum : public ISymbol {
	
	/** The symbol declared in the structure (basically the attributes) */
	Table _table;

	/** The type of the enum */
	syntax::Expression _type;

	/** The values of the enumerations */
	const std::vector <syntax::Expression> _fields;

	/** Set at validation time, to prevent multiple time validation of the same symbol */
	generator::Generator _gen;
	
    private :

	friend Symbol;

	Enum ();

	Enum (const lexing::Word & name, const std::vector <syntax::Expression> & fields, const syntax::Expression & type);

	Enum (const Enum & other);
	
    public :

	static Symbol init (const lexing::Word & name, const std::vector<syntax::Expression> & fields, const syntax::Expression & type);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	void insert (const Symbol & sym) override;

	void insertTemplate (const Symbol & sym) override;

	void replace (const Symbol & sym) override;	

	std::vector <Symbol> get (const std::string & name) const override;

	std::vector <Symbol> getPublic (const std::string & name) const override;

	std::vector <Symbol> getLocal (const std::string & name) const override;

	bool equals (const Symbol & other) const override;

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


