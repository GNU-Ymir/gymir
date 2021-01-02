#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    /**
     * \struct TemplateChecker 
     * A template checker is a syntaxic intrisic for validate template expression at compile time
     * template!X (Y)
     */
    class TemplateChecker : public IExpression {
		
	/** The template parameter of the declaration */
	std::vector <Expression> _parameters;

	/** The expression call use for template specialization */
	std::vector <Expression> _calls;

    private :

	friend Expression; // Needed for dynamic casting

	TemplateChecker ();
	
	TemplateChecker (const lexing::Word & loc, const std::vector <Expression> & calls, const std::vector <Expression> & params);
	
    public :

	/**
	 * \brief Create an initialized template 
	 * \param params the template parameters
	 * \param content the content declaration
	 */
	static Expression init (const lexing::Word & loc, const std::vector <Expression> & calls, const std::vector <Expression> & params);
	
	const std::vector <Expression> & getParameters () const;

	const std::vector <Expression> & getCalls () const;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;
	
    };    

}
