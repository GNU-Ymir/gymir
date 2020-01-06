#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <vector>

namespace semantic {

    /**
     * \struct Function
     * A function is the symbolic representation of a function 
     */
    class Function : public ISymbol {

	/** The symbol table associated to the module */
	Table _table;

	/** The syntaxic function */
	syntax::Declaration _content;

	/** Is this function pure ? (it means purely functional, without any side effect)*/
	bool _isPure;

	/** Is this an inlinable function ?*/
	bool _isInline;

	/** Is this function safe ? (it means cannot crash the program) */
	bool _isSafe;

	/** Is this function variadic */
	bool _isVariadic;

	/**
	 * Is this function marked override
	 */
	bool _isOver = false;
	
	std::string _externLanguage;
	
    private :

	friend Symbol;

	Function ();

	Function (const lexing::Word & name, const syntax::Function & func);

	Function (const Function & other);
	
    public :

	static Symbol init (const lexing::Word & name, const syntax::Function & func);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	void insert (const Symbol & sym) override;

	std::vector<Symbol> getTemplates () const override;
	
	void insertTemplate (const Symbol & sym) override;

	void replace (const Symbol & sym) override;	

	std::vector <Symbol> getLocal (const std::string & name) const override;
	
	std::vector <Symbol> get (const std::string & name) const override;

	std::vector <Symbol> getPublic (const std::string & name) const override;

	bool equals (const Symbol & other) const override;

	void isPure (bool is);

	void isInline (bool is);

	void isSafe (bool is);

	void isOver (bool is);
	
	bool isVariadic () const;

	bool isOver () const;
	
	const syntax::Function & getContent () const;

	/**
	 * \brief Override the space name, to avoid spacing the name of main function
	 */
	std::string getRealName () const override;

	std::string getMangledName () const override;
	
	/**
	 * \brief Change the external language (for external function declaration)
	 */
	void setExternalLanguage (const std::string & name);
	
	/**
	 * \return the external language of the declaration
	 */
	const std::string & getExternalLanguage () const;
	
	std::string formatTree (int padd) const override;

	
    };    

}
