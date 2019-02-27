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
	
    public :

	/**
	 * \brief Create an empty template
	 */
	static Declaration init ();

	/**
	 * \brief Create a template from another one
	 * \param tmpl the template to copy
	 */
	static Declaration init (const Template & tmpl);

	/**
	 * \brief Create an initialized template 
	 * \param params the template parameters
	 * \param content the content declaration
	 */
	static Declaration init (const std::vector <Expression> & params, const Declaration & content);

	
	/**
	 * \brief Create an initialized template 
	 * \param params the template parameters
	 * \param content the content declaration
	 */
	static Declaration init (const std::vector <Expression> & params, const Declaration & content, const Expression & test);

	/**
	 * \brief Mandatory function used for proxy polymoprhism system
	 */
	Declaration clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
	
	/**
	 * \brief Add a new template parameter
	 * \param param the template param
	 */
	void addParameter (const Expression & param);

	/**
	 * \brief Change the content of the template
	 */
	void setContent (const Declaration & content);
    };    

}
