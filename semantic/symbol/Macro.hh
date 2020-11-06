#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/semantic/Generator.hh>

namespace semantic {

    class Macro : public ISymbol {

	/** The symbol declared in the class (the attributes, methods ...) */
	std::shared_ptr <Table> _table;
	
	std::vector <std::string> _skips;
	
    private :

	friend Symbol;

	Macro ();

	Macro (const lexing::Word & name, const	std::vector <std::string> & skips);

    public : 

	static Symbol init (const lexing::Word & name, const std::vector <std::string> & skips);

	bool isOf (const ISymbol * type) const override;

	bool equals (const Symbol & other, bool parent = true) const override;

	void insert (const Symbol & sym) override;

	void insertTemplate (const Symbol & sym) override;

	std::vector<Symbol> getTemplates () const override;
	
	void replace (const Symbol & sym) override;	
	
	std::vector <Symbol> get (const std::string & name) const override;
	
	std::vector <Symbol> getPublic (const std::string & name) const override;

	std::vector <Symbol> getLocal (const std::string & name) const override;

	const std::vector <Symbol> & getAllInner () const;
	
	std::string formatTree (int padd) const override;
    };
    
}