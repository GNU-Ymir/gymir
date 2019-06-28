#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>

namespace semantic {

    /**
     * \struct Template
     * Definition of a template symbol
     * A template symbol posses parameters, and a syntaxic definition
     * Once a template is solved it's definition is transformed and declared
     */
    class Template : public ISymbol {

	std::vector <syntax::Expression> _params;

	syntax::Declaration _decl;

    private : 

	friend Symbol;
	
	Template ();
	
	Template (const lexing::Word & loc, const std::vector<syntax::Expression> & params, const syntax::Declaration & decl);
	
    public :

	static Symbol init (const lexing::Word & loc, const std::vector<syntax::Expression> & params, const syntax::Declaration & decls);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	bool equals (const Symbol & other) const override;

	const syntax::Declaration & getDeclaration () const;

	const std::vector<syntax::Expression> & getParams () const;
	
	std::string formatTree (int padd) const override;
	
    };
        
}
