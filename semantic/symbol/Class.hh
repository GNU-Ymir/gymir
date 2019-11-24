#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/Table.hh>

namespace semantic {

    class Class : public ISymbol {

	/** The symbol declared in the class (the attributes, methods ...) */
	Table _table;

	/** The ancestor of the class */
	syntax::Expression _ancestor;

    private :

	friend Symbol;

	Class ();

	Class (const lexing::Word & name, const syntax::Expression & ancestor);

	Class (const Class & other);
	
    public :
	
	static Symbol init (const lexing::Word & name, const syntax::Expression & ancestor);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	void insert (const Symbol & sym) override;

	void insertTemplate (const Symbol & sym) override;

	std::vector<Symbol> getTemplates () const override;
	
	void replace (const Symbol & sym) override;	
	
	std::vector <Symbol> get (const std::string & name) const override;
	
	std::vector <Symbol> getPublic (const std::string & name) const override;

	std::vector <Symbol> getLocal (const std::string & name) const override;
	
	bool equals (const Symbol & other) const override;

	std::string formatTree (int padd) const override;
	
    };
    
}
