#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/semantic/Generator.hh>

namespace semantic {

    class Macro : public ISymbol {

	/** The symbol declared in the class (the attributes, methods ...) */
	std::shared_ptr <Table> _table;
		
    private :

	friend Symbol;

	Macro ();

	Macro (const lexing::Word & name, const std::string & comments);

    public : 

	static Symbol init (const lexing::Word & name, const std::string & comments);

	bool equals (const Symbol & other, bool parent = true) const override;

	void insert (const Symbol & sym) override;

	void insertTemplate (const Symbol & sym) override;

	void getTemplates (std::vector<Symbol> & rets) const override;
	
	void replace (const Symbol & sym) override;	
	
	void get (const std::string & name, std::vector <Symbol> & rets) const override;
	
	void getPublic (const std::string & name, std::vector <Symbol> & rets) const override;

	void getLocal (const std::string & name, std::vector <Symbol> & rets) const override;

	const std::vector <Symbol> & getAllInner () const;
	
	std::string formatTree (int padd) const override;
    };
    
}
