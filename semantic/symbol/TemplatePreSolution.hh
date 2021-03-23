#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>

namespace semantic {

    /**
     * \struct TemplatePreSolution
     * Definition of a template symbol
     * A template symbol posses parameters, and a syntaxic definition
     * Once a template is solved it's definition is transformed and declared
     */
    class TemplatePreSolution : public ISymbol {
	
	syntax::Declaration _decl;

	std::vector <syntax::Expression> _params;
	
	/** The template parameters performed by a previous template specialiation (but incomplete) Cf TemplatePreSolutionVisitor::validateFromExplicit:(else if syntaxTempl.size () != 0) */
	std::map <std::string, syntax::Expression> _mapping;

	/** The order of _alreadyDone */
	std::vector <std::string> _nameOrder;

	std::weak_ptr <ISymbol> _templateRef;
	
    private : 

	friend Symbol;
	
	TemplatePreSolution ();
	
	TemplatePreSolution (const lexing::Word & loc, const std::string & comments,const std::vector <syntax::Expression> & params, const syntax::Declaration & decls, const std::map <std::string, syntax::Expression> & mapping, const std::vector <std::string> & nameOrder, const Symbol & referent);
	
    public :

	static Symbol init (const lexing::Word & loc, const std::string & comments, const std::vector <syntax::Expression> & params, const syntax::Declaration & decls, const std::map <std::string, syntax::Expression> & mapping, const std::vector <std::string> & nameOrder, const Symbol & referent);

	bool equals (const Symbol & other, bool equals) const override;

	const syntax::Declaration & getDeclaration () const;

	const std::vector <syntax::Expression> & getTemplateParams () const;
	
	const std::vector <std::string> & getNameOrder () const;
	
	const std::map <std::string, syntax::Expression> & getMapping () const;
	
	std::string formatTree (int padd) const override;

	Symbol getTemplateReferent () const;
		
    };
        
}
