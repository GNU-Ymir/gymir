#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Enum.hh>
#include <vector>

namespace semantic {

    class Enum : public ISymbol {
	
	/** The symbol declared in the structure (basically the attributes) */
	Table _table;

	/** The type of the enum */
	syntax::Expression _type;
	
    private :

	friend Symbol;

	Enum ();

	Enum (const lexing::Word & name, const syntax::Expression & type);

	Enum (const Enum & other);
	
    public :

	static Symbol init (const lexing::Word & name, const syntax::Expression & type);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	void insert (const Symbol & sym) override;

	void replace (const Symbol & sym) override;	

	std::vector <Symbol> get (const std::string & name) const override;

	std::vector <Symbol> getLocal (const std::string & name) const override;

	bool equals (const Symbol & other) const override;

	std::string formatTree (int padd) const override;
	
    };

}


