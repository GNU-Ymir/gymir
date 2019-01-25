#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    /**
     * \struct TemplateCall 
     * A template is formed from any expression that can possess template parameters (a var only)
     */
    class TemplateCall : public IExpression {
		
	/** The template parameter of the declaration */
	std::vector <Expression> _parameters;

	/** The content of the template block */
	Expression _content;

    private :

	friend Expression; // Needed for dynamic casting

	TemplateCall ();
	
	TemplateCall (const lexing::Word & loc);
	
    public :

	/**
	 * \brief Create an initialized template 
	 * \param params the template parameters
	 * \param content the content declaration
	 */
	static Expression init (const lexing::Word & loc, const std::vector <Expression> & params, const Expression & content);

	/**
	 * \brief Mandatory function used for proxy polymoprhism system
	 */
	Expression clone () const override;

	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IExpression * type) const override;
	
	/**
	 * \brief Add a new template parameter
	 * \param param the template param
	 */
	void addParameter (const Expression & param);

	/**
	 * \brief Change the content of the template
	 */
	void setContent (const Expression & content);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };    

}
