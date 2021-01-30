#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>
#include <vector>


namespace semantic {

    /**
     * \struct Impl
     * A impl is the implementation of a trait in a class
     */
    class Impl : public ISymbol {

	/** The symbol table associated to the implem */
	std::shared_ptr<Table> _table;
	
	syntax::Expression _trait;
	
    private :

	friend Symbol;

	Impl ();
	
	Impl (const lexing::Word & name, const std::string & comments, const syntax::Expression & trait, bool isWeak);

	Impl (const Impl & mod);
	
    public :

	static Symbol init (const lexing::Word & name, const std::string & comments, const syntax::Expression & trait, bool isWeak);
	
	void insert (const Symbol & sym) override;
	
	void replace (const Symbol & sym) override;

	void get (const std::string & name, std::vector <Symbol> & rets) const override;

	void  getPublic (const std::string & name, std::vector <Symbol> & rets) const override;

	void getLocal (const std::string & name, std::vector <Symbol> & rets) const override;

	void getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const override;

	const std::vector <Symbol> & getAllInner () const;
	
	bool equals (const Symbol & other, bool parent = true) const override;

	/**
	 * \return a formated string of the hierarchy of the modules and all declared symbols
	 */
	std::string formatTree (int padd) const override;

	const syntax::Expression & getTrait () const;

	/** The name of the impl must be ignored */
	std::string getRealName () const override;
	
    };
    
}
