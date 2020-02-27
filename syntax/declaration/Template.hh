#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    /**
     * \struct Template 
     * A template is formed from any declaration that can possess template parameters
     * For exemple function, module, structure ...
     * A function with template parameters will be transform (at syntax time) to a template containing a function
     * Done this way we can ensure that every template element will be treated the same way
     */
    class Template : public IDeclaration {

	/** The template parameter of the declaration */
	std::vector <Expression> _parameters;

	/** The content of the template block */
	Declaration _content;

	/** The test of the template */
	Expression _test;
	
    private :

	friend Declaration; // Needed for dynamic casting
	
	Template ();

	Template (const lexing::Word & loc, const std::vector <Expression> & params, const Declaration & content, const Expression & test);
	
    public :
	
	/**
	 * \brief Create an initialized template 
	 * \param params the template parameters
	 * \param content the content declaration
	 */
	static Declaration init (const lexing::Word & loc, const std::vector <Expression> & params, const Declaration & content, const Expression & test);

	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
	
	/**
	 * \return the list of template parameters of the declarations
	 */
	const std::vector <Expression> & getParams () const;

	/**
	 * \return the content of the declaration
	 */
	const Declaration & getContent () const;

	/**
	 * \return the test of the template (or empty)
	 */
	const Expression & getTest () const;
	
    };    

}
