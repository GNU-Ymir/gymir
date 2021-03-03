#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <vector>


namespace semantic {

    /**
     * \struct Module
     * A module is a part of the program 
     * It has different access to foreign module
     */
    class Module : public ISymbol {

	/** The symbol table associated to the module */
	std::shared_ptr<Table> _table;

	/** set to true, if the module was imported  */
	bool _isExtern;
	
    private :

	friend Symbol;

	Module ();
	
	Module (const lexing::Word & name, const std::string & comments, bool isWeak, bool isTrusted);

	Module (const Module & mod);
	
    public :

	static Symbol init (const lexing::Word & name, const std::string & comments, bool isWeak, bool isTrusted);
	
	void insert (const Symbol & sym) override;

	void insertTemplate (const Symbol & sym) override;

	void getTemplates (std::vector<Symbol> & ret) const override;
	
	// void replace (const Symbol & sym) override;

	void get (const std::string & name, std::vector <Symbol> & ret) const override;

	void getPublic (const std::string & name, std::vector <Symbol> & ret) const override;

	void getLocal (const std::string & name, std::vector <Symbol> & ret) const override;

	void getLocalPublic (const std::string & name, std::vector <Symbol> & ret) const override;

	const std::vector <Symbol> & getAllLocal () const;
	
	bool equals (const Symbol & other, bool parent = true) const override;

	/**
	 * \return Does this module comes from external file ?
	 */
	bool isExtern () const;

	/** 
	 * \brief if is is set to true, this module comes from an external file
	 */
	void isExtern (bool is);

	/**
	 * \return a formated string of the hierarchy of the modules and all declared symbols
	 */
	std::string formatTree (int padd) const override;
	
    };
    
}
