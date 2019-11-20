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

	syntax::Expression _test;
	
	syntax::Declaration _decl;

	/** The template parameters performed by a previous template specialiation (but incomplete) Cf TemplateVisitor::validateFromExplicit:(else if syntaxTempl.size () != 0) */
	std::map <std::string, syntax::Expression> _alreadyDone;

	/** The order of _alreadyDone */
	std::vector <std::string> _nameOrder;

	std::vector <syntax::Expression> _previousParams;
	
    private : 

	friend Symbol;
	
	Template ();
	
	Template (const lexing::Word & loc, const std::vector<syntax::Expression> & params, const syntax::Declaration & decl, const syntax::Expression & test, const std::vector<syntax::Expression> & previousParams);
	
    public :

	static Symbol init (const lexing::Word & loc, const std::vector<syntax::Expression> & params, const syntax::Declaration & decls, const syntax::Expression & test, const std::vector<syntax::Expression> & previousParams);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;

	bool equals (const Symbol & other) const override;

	const syntax::Declaration & getDeclaration () const;

	const std::vector<syntax::Expression> & getParams () const;

	const std::vector<syntax::Expression> & getPreviousParams () const;	

	const syntax::Expression & getTest () const;

	/**
	 * \brief Set the previous template decl
	 */
	void setPreviousSpecialization (const std::map <std::string, syntax::Expression> & previous);

	const std::map <std::string, syntax::Expression> & getPreviousSpecialization () const;
	
	const std::vector <std::string> & getSpecNameOrder () const;

	void setSpecNameOrder (const std::vector <std::string> & nameOrder);	
	
	std::string formatTree (int padd) const override;

	std::string prettyString () const;
	
    };
        
}
