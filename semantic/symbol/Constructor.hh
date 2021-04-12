#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Constructor.hh>
#include <ymir/semantic/Generator.hh>
#include <vector>

namespace semantic {

    /**
     * \struct Constructor
     * A function is the symbolic representation of a function 
     */
    class Constructor : public ISymbol {

	/** The symbol table associated to the module */
	std::shared_ptr<Table> _table;

	/** The syntaxic function */
	syntax::Declaration _content;

	/** The prototype of the function, set at validation time */
	generator::Generator _proto;
	
	std::weak_ptr <ISymbol> _class;	      
	
	std::vector <syntax::Expression> _throwers;
		
    private :

	friend Symbol;

	Constructor ();

	Constructor (const lexing::Word & name, const std::string & comments, const syntax::Constructor & func, bool isWeak);

	Constructor (const Constructor & other);
	
    public :

	static Symbol init (const lexing::Word & name, const std::string & comments, const syntax::Constructor & func, bool isWeak);

	void insert (const Symbol & sym) override;

	void getTemplates (std::vector<Symbol> & ret) const override;
	
	void insertTemplate (const Symbol & sym) override;

	// void replace (const Symbol & sym) override;	

	void getLocal (const std::string & name, std::vector <Symbol> & ret) const override;
	
	void getLocalPublic (const std::string & name, std::vector <Symbol> & ret) const override;
	
	void get (const std::string & name, std::vector <Symbol> & ret) const override;

	void getPublic (const std::string & name, std::vector <Symbol> & ret) const override;

	bool equals (const Symbol & other, bool parent) const override;

	const syntax::Constructor & getContent () const;

	/**
	 * \brief Override the space name, to avoid spacing the name of main function
	 */
	std::string computeRealName () const override;

	std::string computeMangledName () const override;
	
	std::string formatTree (int padd) const override;

	void setClass (const Symbol & sym);

	void setThrowers (const std::vector <syntax::Expression> & thrs);

	const std::vector <syntax::Expression> & getThrowers () const;
	
	Symbol getClass () const;

	const lexing::Word & getRename () const;
	
	/**
	 * \return the list of custom attributs
	 */
	const std::vector <lexing::Word> & getCustomAttributes () const;


	
	/**
	 * \brief This information is set at validation time
	 * \return the generator (function prototype)
	 */
	const generator::Generator & getGenerator () const;

	/**
	 * \brief This information is set at validation time
	 * \brief set the generator (function prototype)
	 */
	void setGenerator (const generator::Generator & gen);
	
    };    

}
