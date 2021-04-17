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
	std::shared_ptr <Table> _table;

	std::vector <syntax::Expression> _templs;
	
	std::map <std::string, syntax::Expression> _params;
	
	std::vector <std::string> _nameOrder;
	
    private : 

	friend Symbol;
	
	TemplateSolution ();

	TemplateSolution (const TemplateSolution & mod);
	
	TemplateSolution (const lexing::Word & loc, const std::string & comments, const std::vector <syntax::Expression> & templs, const std::map<std::string, syntax::Expression> & params, const std::vector <std::string> & nameOrders, bool isWeak);
	
    public :

	static Symbol init (const lexing::Word & loc, const std::string & comments, const std::vector <syntax::Expression> & templs, const std::map<std::string, syntax::Expression> & params, const std::vector <std::string> & nameOrders, bool isWeak);

	void insert (const Symbol & sym) override;

	// void replace (const Symbol & sym) override;

	void get (const std::string & name, std::vector <Symbol> & ret) const override;

	void getPublic (const std::string & name, std::vector <Symbol> & ret) const override;

	void getLocal (const std::string & name, std::vector <Symbol> & ret) const override;
	
	void getLocalPublic (const std::string & name, std::vector <Symbol> & ret) const override;
	
	const std::vector <Symbol> & getAllLocal () const;
	
	bool equals (const Symbol & other, bool parent = true) const override;
	
	std::string formatTree (int padd) const override;

	std::string computeRealName () const override;
	
	std::string getSolutionName () const;

	std::string computeMangledName () const override;

	void pruneTable ();

	const std::vector<syntax::Expression> & getTempls () const;

	const std::map <std::string, syntax::Expression> & getParams () const;
	
    };
        
}
