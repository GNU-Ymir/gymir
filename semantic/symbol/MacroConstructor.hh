#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/Declaration.hh>
#include <vector>

namespace semantic {

    /**
     * \struct MacroConstructor
     * A function is the symbolic representation of a function 
     */
    class MacroConstructor : public ISymbol {

	syntax::Declaration _constr;
	
    private :

	friend Symbol;

	MacroConstructor ();

	MacroConstructor (const lexing::Word & name, const std::string & comments, const syntax::Declaration & cstr);
	
    public :

	static Symbol init (const lexing::Word & name, const std::string & comments, const syntax::Declaration & func);

	bool isOf (const ISymbol * type) const override;

	bool equals (const Symbol & other, bool parent) const override;

	const syntax::Declaration & getContent () const;
	
	std::string formatTree (int padd) const override;
	
    };    

}
