#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>

namespace syntax {

    /**
     * \brief DestructDecl
     * Represent the declaration of a new destructor of tuple
     * \verbatim
     var_decl := '(' inner_var_decl (',' inner_var_decl)* ')' '=' expression
     inner_var_decl := ('ref')? ('const')? ('cte')? Identifier (':' expression)? 
     \endverbatim
     */
    class DestructDecl : public IExpression {

	/** The inner var decls */
	std::vector <Expression> _params;

	/** The value that will be destructed*/
	Expression _value;

	bool _isVariadic;
	
    private :

	friend Expression;

	DestructDecl ();
	
	DestructDecl (const lexing::Word & loc, const std::vector<Expression> & params, const Expression & value, bool isVariadic);

    public :

	/**
	 * \brief Create a new DestructDecl
	 * \param loc the location of the destructor
	 * \param params the list of var declaration
	 * \param value the value to destructure
	 */
	static Expression init (const lexing::Word & loc, const std::vector <Expression> & params, const Expression & value, bool isVariadic);
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the list of var declaration 
	 */
	const std::vector <Expression> & getParameters () const;
	
	/**
	 * \return the value of the declaration
	 */
	const Expression & getValue () const;

	/**
	 * \return is this destruct declaration variadic?
	 */
	bool isVariadic () const;

	std::string prettyString () const override;
	
    };
    
}
