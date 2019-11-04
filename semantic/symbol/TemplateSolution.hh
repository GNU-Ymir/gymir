#pragma once

#include <ymir/semantic/Table.hh>
#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>

namespace semantic {

    /**
     * \struct TemplateSolution
     * Definition of a template symbol that has been resolved
     * A template symbol posses parameters, and a syntaxic definition
     * Once a template is solved it's definition is transformed and declared
     */
    class TemplateSolution : public ISymbol {

	/** The symbol table associated to the template solution */
	Table _table;

	std::vector <syntax::Expression> _templs;
	
	std::map <std::string, syntax::Expression> _params;

    private : 

	friend Symbol;
	
	TemplateSolution ();

	TemplateSolution (const TemplateSolution & mod);
	
	TemplateSolution (const lexing::Word & loc, const std::vector <syntax::Expression> & templs, const std::map<std::string, syntax::Expression> & params);
	
    public :

	static Symbol init (const lexing::Word & loc, const std::vector <syntax::Expression> & templs, const std::map<std::string, syntax::Expression> & params);

	Symbol clone () const override;

	void insert (const Symbol & sym) override;

	void replace (const Symbol & sym) override;

	std::vector <Symbol> get (const std::string & name) const override;

	std::vector <Symbol> getLocal (const std::string & name) const override;
	
	const std::vector <Symbol> & getAllLocal () const;
	
	bool isOf (const ISymbol * type) const override;

	bool equals (const Symbol & other) const override;
	
	std::string formatTree (int padd) const override;

	std::string getRealName () const override;

	std::string getMangledName () const override;

	const std::vector<syntax::Expression> & getTempls () const;

	const std::map <std::string, syntax::Expression> & getParams () const;
	
    };
        
}