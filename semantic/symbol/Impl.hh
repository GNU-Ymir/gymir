#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/Expression.hh>
#include <vector>


namespace semantic {

    /**
     * \struct Impl
     * A impl is the implementation of a trait in a class
     */
    class Impl : public ISymbol {

	/** The symbol table associated to the implem */
	Table _table;

	syntax::Expression _trait;
	
    private :

	friend Symbol;

	Impl ();
	
	Impl (const lexing::Word & name, const syntax::Expression & trait);

	Impl (const Impl & mod);
	
    public :

	static Symbol init (const lexing::Word & name, const syntax::Expression & trait);
	
	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	void insert (const Symbol & sym) override;
	
	void replace (const Symbol & sym) override;

	std::vector <Symbol> get (const std::string & name) const override;

	std::vector <Symbol> getPublic (const std::string & name) const override;

	std::vector <Symbol> getLocal (const std::string & name) const override;

	std::vector <Symbol> getLocalPublic (const std::string & name) const override;

	const std::vector <Symbol> & getAllInner () const;
	
	bool equals (const Symbol & other) const override;

	/**
	 * \return a formated string of the hierarchy of the modules and all declared symbols
	 */
	std::string formatTree (int padd) const override;

	const syntax::Expression & getTrait () const;

	/** The name of the impl must be ignored */
	std::string getRealName () const override;
	
    };
    
}
