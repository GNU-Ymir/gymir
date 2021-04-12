#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/semantic/Generator.hh>

namespace semantic {

    class Trait : public ISymbol {

	/** The symbol declared in the class (the attributes, methods ...) */
	std::shared_ptr<Table> _table;

	
	std::vector <syntax::Expression> _fields;

	/** The list of fields that are marked as private */
	std::vector <std::string> _privates;

	/**
	 * the list of fields that are marked protected 
	 */
	std::vector <std::string> _protected;

	/** The assertion that must be validated within the class */
	std::vector <syntax::Expression> _assertions;

	/** The comments above the assertion (for documentation dumping only) */
	std::vector <std::string> _assertion_comments;
	
	/**
	 * set at validation time, to prevent multiple time validation of the same symbol
	 */
	generator::Generator _gen;
	
    private :

	friend Symbol;

	Trait ();

	Trait (const lexing::Word & name, const std::string & comments, bool isWeak);

	Trait (const Trait & other);
	
    public :
	
	static Symbol init (const lexing::Word & name, const std::string & comments, bool isWeak);

	void insert (const Symbol & sym) override;

	void insertTemplate (const Symbol & sym) override;

	void getTemplates (std::vector<Symbol> & rets) const override;
	
	// void replace (const Symbol & sym) override;	
	
	void get (const std::string & name, std::vector<Symbol> & rets) const override;
	
	void getPublic (const std::string & name, std::vector<Symbol> & rets) const override;

	void getLocal (const std::string & name, std::vector<Symbol> & rets) const override;

	const std::vector <Symbol> & getAllInner () const;
	
	bool equals (const Symbol & other, bool equals = true) const override;

	std::string formatTree (int padd) const override;

	/**
	 * \brief This information is set at validation time
	 * \return the generator (structure prototype)
	 */
	const generator::Generator & getGenerator () const;

	/**
	 * \brief This information is set at validation time
	 * \brief set the generator (structure prototype)
	 */
	void setGenerator (const generator::Generator & gen);

	/**
	 * \brief Add a field in the class declaration
	 */
	void addField (const syntax::Expression & field);

	/**
	 * \brief Mark a field has private
	 */
	void setPrivate (const std::string & name);


	/**
	 * \brief Mark a field has private
	 */
	void setProtected (const std::string & name);
	
	/**
	 * \return the list of fields
	 */
	const std::vector<syntax::Expression> & getFields () const;

	/**
	 * \return true if the field is mark private
	 * \warning if the fields does not exists in the class it will return false
	 * \warning we assume that this function is called oif we are sure that the fields exists in the class
	 */
	bool isMarkedPrivate (const std::string & name) const;

	/**
	 * \return true if the field is mark protected
	 * \warning if the fields does not exists in the class it will return false
	 * \warning we assume that this function is called oif we are sure that the fields exists in the class
	 */
	bool isMarkedProtected (const std::string & name) const;	

	/** The name of the trait must be ignored */
	std::string computeRealName () const override;
	
	/**
	 * \brief Add an assertion to validate in the class
	 */
	void addAssertion (const syntax::Expression & assert);

	/** 
	 * \brief add a comment about the last inserted assertion
	 */
	void addAssertionComments (const std::string & str);

	/**
	 * \return the list of assertion in the class
	 */
	const std::vector <syntax::Expression> & getAssertions () const;
	
	/**
	 * \return the comments on the assertions
	 */
	const std::vector <std::string> & getAssertionComments () const;

    };
    
}
