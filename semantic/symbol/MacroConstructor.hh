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

	/** The symbol table associated to the module */
	std::shared_ptr<Table> _table;
	
	syntax::Declaration _constr;
	
    private :

	friend Symbol;

	MacroConstructor ();

	MacroConstructor (const lexing::Word & name, const std::string & comments, const syntax::Declaration & cstr);

	MacroConstructor (const MacroConstructor & other);
	
    public :

	static Symbol init (const lexing::Word & name, const std::string & comments, const syntax::Declaration & func);

	void insert (const Symbol & sym) override;

	void getTemplates (std::vector<Symbol> & rets) const override;
	
	void insertTemplate (const Symbol & sym) override;

	// void replace (const Symbol & sym) override;	

	void getLocal (const std::string & name, std::vector<Symbol> & rets) const override;

	void getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const override;
	
	void get (const std::string & name, std::vector <Symbol> & rets) const override;

	void getPublic (const std::string & name, std::vector <Symbol> & rets) const override;
	
	bool equals (const Symbol & other, bool parent) const override;

	const syntax::Declaration & getContent () const;
	
	std::string formatTree (int padd) const override;
	
    };    

}
