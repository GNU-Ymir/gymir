#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/generator/Value.hh>


namespace semantic {

    namespace generator {

	/**
	 * \struct TemplateSyntaxList 
	 * Intermediate generator created by template specialisation
	 * It is a syntaxic expression but a fake one, as it only store a list of semantic generator
	 * Unlike TemplateSyntaxList, this class is used when generating a template specialization of a variadic template
	 */
	class TemplateSyntaxList : public syntax::IExpression {

	    std::vector <Generator> _contents;
	    
	private :

	    friend syntax::Expression;

	    TemplateSyntaxList ();

	    TemplateSyntaxList (const lexing::Word& loc, const std::vector<Generator> & gen);

	public : 

	    static syntax::Expression init (const lexing::Word & location, const std::vector<Generator> & gen);

	    void treePrint (Ymir::OutBuffer & stream, int i) const override;
	    

	    /**
	     * \return the list of content wrapped inside this fake syntax expression
	     */
	    const std::vector<Generator>& getContents () const;

	    std::string prettyString () const override;
	    
	};
	
    }    

}

