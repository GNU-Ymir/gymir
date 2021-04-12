#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/semantic/Generator.hh>
#include <vector>

namespace semantic {

    /**
     * \struct Function
     * A function is the symbolic representation of a function 
     */
    class Function : public ISymbol {

	/** The symbol table associated to the module */
	std::shared_ptr<Table> _table;

	/** The syntaxic function */
	syntax::Declaration _content;

       
	/** The prototype of the function, set at validation time */
	generator::Generator _proto;

	/** Is this function @pure ? (it means purely functional, without any side effect)*/
	bool _isPure;

	/** Is this an inlinable function ?*/
	bool _isInline;

	/** Is this function @safe ? (it means cannot crash the program) */
	bool _isSafe = true;

	/** Is this function variadic */
	bool _isVariadic;

	/**
	 * Is this function marked override
	 */
	bool _isOver = false;

	/**
	 * Is this function marked @final
	 */
	bool _isFinal = false;

	std::vector <syntax::Expression> _throwers;
	
	std::string _externLanguage;
	
    private :

	friend Symbol;

	Function ();

	Function (const lexing::Word & name, const std::string & comments, const syntax::Function & func, bool isWeak);

	Function (const Function & other);
	
    public :

	static Symbol init (const lexing::Word & name, const std::string & comments, const syntax::Function & func, bool isWeak);

	void insert (const Symbol & sym) override;

	void getTemplates (std::vector<Symbol> & rets) const override;
	
	void insertTemplate (const Symbol & sym) override;

	// void replace (const Symbol & sym) override;	

	void getLocal (const std::string & name, std::vector<Symbol> & rets) const override;

	void getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const override;
	
	void get (const std::string & name, std::vector <Symbol> & rets) const override;

	void getPublic (const std::string & name, std::vector <Symbol> & rets) const override;

	bool equals (const Symbol & other, bool parent) const override;

	void isPure (bool is);

	void isInline (bool is);

	void isSafe (bool is);

	bool isSafe () const;

	void isOver (bool is);
	
	bool isVariadic () const;

	bool isOver () const;

	bool isMethod () const;

	bool isFinal () const;

	void isFinal (bool is);

	void setThrowers (const std::vector <syntax::Expression> & throwers);

	const std::vector <syntax::Expression> & getThrowers () const;
	
	const syntax::Function & getContent () const;

	/**
	 * \brief Override the space name, to avoid spacing the name of main function
	 */
	std::string computeRealName () const override;

	std::string computeMangledName () const override;
	
	/**
	 * \brief Change the external language (for external function declaration)
	 */
	void setExternalLanguage (const std::string & name);
	
	/**
	 * \return the external language of the declaration
	 */
	const std::string & getExternalLanguage () const;
	
	std::string formatTree (int padd) const override;

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
