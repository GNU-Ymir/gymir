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

	std::string _externLanguage;
	
    private :

	friend Symbol;

	VarDecl ();

	VarDecl (const lexing::Word & name, const std::string & comments, const std::vector <syntax::DecoratorWord> & decos, const syntax::Expression & type, const syntax::Expression & value, bool isWeak);

    public : 

	static Symbol init (const lexing::Word & name, const std::string & comments, const std::vector <syntax::DecoratorWord> & decos, const syntax::Expression & type, const syntax::Expression & value, bool isWeak);

	bool equals (const Symbol & other, bool parent = true) const override;

	const syntax::Expression & getValue () const;
	
	const syntax::Expression & getType () const;

	void setGenerator (const generator::Generator & gen);

	const generator::Generator & getGenerator () const;
	
	const std::vector <syntax::DecoratorWord> & getDecorators () const;

	/**
	 * \brief Change the external language (for external global var declaration)
	 */
	void setExternalLanguage (const std::string & name);
	
	/**
	 * \return the external language of the declaration
	 */
	const std::string & getExternalLanguage () const;

	bool isExtern () const;
	
	std::string formatTree (int padd) const override;
    };
    
}
