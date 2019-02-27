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
	
	Expression _content;

    protected :

	friend Expression;

	Scope ();
	
	Scope (const lexing::Word & loc);

    public :

	/**
	 * \brief Create a new Scope 
	 * \param location the location of the scope
	 * \param content the content of the scope
	 */
	static Expression init (const lexing::Word & location, const Expression & content);

	virtual Expression clone () const override;

	virtual bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
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

	/** The types of the catching variables */
	std::vector <Expression> _types;

	/** The content for each catching variables */
	std::vector <Expression> _contents;

    private :

	friend Expression;

	ScopeFailure ();
	
	ScopeFailure (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & location, const std::vector <Expression> &types, const std::vector <Expression> & contents);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };
    
}
