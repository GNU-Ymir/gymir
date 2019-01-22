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
	Table _table;

    private :

	friend Symbol;

	Module ();
	
	Module (const lexing::Word & name);

    public :

	static Symbol init (const lexing::Word & name);
	
	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	void insert (const Symbol & sym) override;

	std::vector <Symbol> get (const std::string & name) const;

	const Symbol & getLocal (const std::string & name) const override;

	const std::vector <Symbol> & getAllLocal () const;
	
	bool equals (const Symbol & other) const override;
	
    };
    
}
