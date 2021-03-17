#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/Generator.hh>

namespace semantic {

    class Aka : public ISymbol {

	
	/** The content of the alias */
	syntax::Expression _value;

	generator::Generator _gen;

    private :

	friend Symbol;

	Aka ();

	Aka (const lexing::Word & name, const std::string & comments, const syntax::Expression & expr, bool isWeak);

    public : 

	static Symbol init (const lexing::Word & name, const std::string & comments, const syntax::Expression & expr, bool isWeak);

	bool equals (const Symbol & other, bool parent = true) const override;

	const syntax::Expression & getValue () const;

	void setGenerator (const generator::Generator & gen);

	const generator::Generator & getGenerator () const;
	
	std::string formatTree (int padd) const override;
    };
    
}
