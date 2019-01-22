#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/Table.hh>

namespace semantic {

    class Class : public ISymbol {

	/** The symbol declared in the class (the attributes, methods ...) */
	Table _table;

	/** The list of overloading structures */
	std::vector <Symbol> _overload;

	/** The ancestor of the class */
	syntax::Expression _ancestor;

    private :

	friend Symbol;

	Class ();

	Class (const lexing::Word & name, const syntax::Expression & ancestor);

    public :
	
	static Symbol init (const lexing::Word & name, const syntax::Expression & ancestor);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	void insert (const Symbol & sym) override;

	std::vector <Symbol> get (const std::string & name) const override;

	const Symbol & getLocal (const std::string & name) const override;
	
	const std::vector <Symbol> & getOverloading () const;

	void setOverloading (const std::vector <Symbol> & overs) ;

	bool equals (const Symbol & other) const override;

	
    };
    
}
