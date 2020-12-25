#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>
#include <ymir/semantic/Generator.hh>

namespace semantic {

    class VarDecl : public ISymbol {

	/** the decorator of the var */
	std::vector <syntax::DecoratorWord> _decos;
	
	/** The type of the decl */
	syntax::Expression _type;
	
	/** The content of the decl */
	syntax::Expression _value;

	/** The generator (created at validation time) */
	generator::Generator _gen;
	
    private :

	friend Symbol;

	VarDecl ();

	VarDecl (const lexing::Word & name, const std::string & comments, const std::vector <syntax::DecoratorWord> & decos, const syntax::Expression & type, const syntax::Expression & value, bool isWeak);

    public : 

	static Symbol init (const lexing::Word & name, const std::string & comments, const std::vector <syntax::DecoratorWord> & decos, const syntax::Expression & type, const syntax::Expression & value, bool isWeak);

	bool isOf (const ISymbol * type) const override;

	bool equals (const Symbol & other, bool parent = true) const override;

	const syntax::Expression & getValue () const;
	
	const syntax::Expression & getType () const;

	void setGenerator (const generator::Generator & gen);

	const generator::Generator & getGenerator () const;
	
	const std::vector <syntax::DecoratorWord> & getDecorators () const;
	
	std::string formatTree (int padd) const override;
    };
    
}
