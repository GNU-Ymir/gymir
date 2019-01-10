#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Scope
     * A scope is a event that append at the end of a scope 
     * It can be a failure, a success or just a standard exit
     * \verbatim
     scope := 'on' Identifier '=>' expression
     \endverbatim
     */
    class Scope : public IExpression {
    protected:
	
	lexing::Word _location;

	Expression _content;

    protected :

	friend Expression;

	Scope ();

    public :

	/**
	 * \brief Make a copy of a scope 
	 * \param scope the expression to copy
	 */
	static Expression init (const Scope & scope);

	/**
	 * \brief Create a new Scope 
	 * \param location the location of the scope
	 * \param content the content of the scope
	 */
	static Expression init (const lexing::Word & location, const Expression & content);

	virtual Expression clone () const override;

	virtual bool isOf (const IExpression * type) const override;
	
    };    

    /**
     * \struct ScopeFailure 
     * A scope failure is a special scope event, that append when exception is raised
     * \verbatim
     scope_failure := 'on' 'failure' (('=>' expression) | '{' (typed_var expression)* '}')
     typed_var := Identifier ':' expression
     \endverbatim
     */
    class ScopeFailure : public Scope {

	/** The name of the catching variables */
	std::vector <lexing::Word> _names;

	/** The types of the catching variables */
	std::vector <Expression> _types;

	/** The content for each catching variables */
	std::vector <Expression> _contents;


    private :

	friend Expression;

	ScopeFailure ();

    public :

	/**
	 * \brief Make a copy of a scope failure 
	 * \param scope the scope to copy
	 */
	static Expression init (const ScopeFailure & scope);


	static Expression init (const lexing::Word & location, const std::vector <lexing::Word> & names, const std::vector <Expression> &types, const std::vector <Expression> & contents);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };
    
}
