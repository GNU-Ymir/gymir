#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Struct.hh>
#include <vector>

namespace semantic {

    class Struct : public ISymbol {
	
	/** The symbol declared in the structure (basically the attributes) */
	Table _table;

	/** The list of overloading structures */
	std::vector <Symbol> _overload;

	/** Is this structure data packed (<=> _isUnion == false) */
	bool _isPacked;

	/** Is this structure an union */
	bool _isUnion;

    private :

	friend Symbol;

	Struct ();

	Struct (const lexing::Word & name);

    public :

	static Symbol init (const lexing::Word & name);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	void insert (const Symbol & sym) override;

	std::vector <Symbol> get (const std::string & name) const override;

	const Symbol & getLocal (const std::string & name) const override;
	
	const std::vector <Symbol> & getOverloading () const;

	void setOverloading (const std::vector <Symbol> & overs) ;

	bool equals (const Symbol & other) const override;

	void isPacked (bool isPacked);

	bool isPacked () const;

	void isUnion (bool isUnion);

	bool isUnion () const;
	
    };

}

