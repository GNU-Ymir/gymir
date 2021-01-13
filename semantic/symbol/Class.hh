#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/semantic/Generator.hh>

namespace semantic {

    class Class : public ISymbol {
	
	/** The symbol declared in the class (the attributes, methods ...) */
	std::shared_ptr <Table> _table;

	/** The ancestor of the class */
	syntax::Expression _ancestor;

	/** The fields of the class */
	std::vector <syntax::Expression> _fields;

	/** The assertion that must be validated within the class */
	std::vector <syntax::Expression> _assertions;

	/** The comments above the assertion (for documentation dumping only) */
	std::vector <std::string> _assertion_comments;
	
	/** The comments about the field of the class */
	std::map <std::string, std::string> _field_comments;
	
	/** The list of fields that are marked as private */
	std::set <std::string> _privates;

	/**
	 * the list of fields that are marked protected 
	 */
	std::set <std::string> _protected;
	
	/**
	 * set at validation time, to prevent multiple time validation of the same symbol
	 */
	generator::Generator _gen;

	/**
	 * Set at validation time, it is the typeinfo of the class
	 */
	generator::Generator _typeInfo;

	/** The list of function that were declared in a traits, but
	    not overriden by the class 
	    They were put directly in the class as they were, and will be validate likewise
	    This vector is filled at validation time
	*/
	std::vector <Symbol> _addMethods;
	
	/**
	 * Is this class abstract (not instanciable, and doesn't need all its methods to have body)
	 */
	bool _isAbstract = false;

	/**
	 * Is this class final (not inheritable)
	 */
	bool _isFinal = false;
	
    private :

	friend Symbol;

	Class ();

	Class (const lexing::Word & name, const std::string & comments, const syntax::Expression & ancestor, bool isWeak);

	Class (const Class & other); // never used
	
    public :
	
	static Symbol init (const lexing::Word & name, const std::string & comments, const syntax::Expression & ancestor, bool isWeak);


	void insert (const Symbol & sym) override;

	void insertTemplate (const Symbol & sym) override;

	void getTemplates (std::vector<Symbol> & ret) const override;
	
	void replace (const Symbol & sym) override;	
	
	void get (const std::string & name, std::vector <Symbol> & ret) const override;
	
	void getPublic (const std::string & name, std::vector <Symbol> & ret) const override;

	void getLocal (const std::string & name, std::vector <Symbol> & ret) const override;

	const std::vector <Symbol> & getAllInner () const;
	
	bool equals (const Symbol & other, bool parent = true) const override;

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
	 * \brief This information is set at validation time
	 * \return the typeinfo 
	 */
	const generator::Generator & getTypeInfo () const;

	/**
	 * \brief This information is set at validation time
	 * \brief set the typeinfo 
	 */
	void setTypeInfo (const generator::Generator & gen);

	/**
	 * \brief Add a field in the class declaration
	 */
	void addField (const syntax::Expression & field);

	/**
	 * \brief Register the comment of a field for later documentation dumping
	 */
	void setFieldComment (const std::string & name, const std::string & comment);

	/**
	 * \return the comments on a field declaration
	 */
	std::string getFieldComments (const std::string & name) const;

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

	/**
	 * Set the class to abstract if is
	 */
	void isAbs (bool is);

	/**
	 * \return is this class abstract?
	 */
	bool isAbs () const;

	/**
	 * Set the class to final if /is/
	 */
	void isFinal (bool is);

	/**
	 * \return Is this class abstract
	 */
	bool isFinal () const;
	
	/**
	 * return the ancestor of the class (might be empty)
	 */
	const syntax::Expression & getAncestor () const;

	/**
	 * \return the list of added methods (trait but not overriden)
	 */
	const std::vector <Symbol> & getAddMethods () const;

	/**
	 * \brief Set the list of methods that were directly imported from trait
	 * \brief All those methods were not declared in the class, so they are not accessible in its symbol table
	 */
	void setAddMethods (const std::vector <Symbol> & methods);
	
    };
    
}
