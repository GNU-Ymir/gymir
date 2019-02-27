#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Struct.hh>
#include <vector>

namespace semantic {

    class Struct : public ISymbol {
	
	/** The symbol declared in the structure (basically the attributes) */
	Table _table;

	/** Is this structure data packed (<=> _isUnion == false) */
	bool _isPacked;

	/** Is this structure an union */
	bool _isUnion;

    private :

	friend Symbol;

	Struct ();

	Struct (const lexing::Word & name);

	Struct (const Struct & str);
	
    public :

	static Symbol init (const lexing::Word & name);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	void insert (const Symbol & sym) override;

	void replace (const Symbol & sym) override;	
	
	std::vector <Symbol> get (const std::string & name) const override;

	std::vector <Symbol> getLocal (const std::string & name) const override;

	bool equals (const Symbol & other) const override;

	void isPacked (bool isPacked);

	bool isPacked () const;

	void isUnion (bool isUnion);

	bool isUnion () const;
	
	std::string formatTree (int padd) const override;
    };

}


