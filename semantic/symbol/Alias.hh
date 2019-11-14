#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/Generator.hh>

namespace semantic {

    class Alias : public ISymbol {

	
	/** The content of the alias */
	syntax::Expression _value;

	generator::Generator _gen;

    private :

	friend Symbol;

	Alias ();

	Alias (const lexing::Word & name, const syntax::Expression & expr);

    public : 

	static Symbol init (const lexing::Word & name, const syntax::Expression & expr);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	bool equals (const Symbol & other) const override;

	const syntax::Expression & getValue () const;

	void setGenerator (const generator::Generator & gen);

	const generator::Generator & getGenerator () const;
	
	std::string formatTree (int padd) const override;
    };
    
}
