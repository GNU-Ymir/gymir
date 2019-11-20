#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>

namespace syntax {

    /**
     * \brief VarDecl
     * Represent the declaration of a new variable
     * Multiple variable declaration in one line are splitted into multiple VarDecl instance
     * It can also be a part of a tupleDest (in that case _value is always empty ())
     * \verbatim
     var_decl := 'let' inner_var_decl (',' inner_var_decl)* ';'
     inner_var_decl := ('ref')? ('const')? ('cte')? Identifier (':' expression)? ('=' expression)?
     \endverbatim
     */
    class VarDecl : public IExpression {

	/** the name of the variable */
	lexing::Word _name;

	/** The type of the variable */
	Expression _type;

	/** The value of the variable */
	Expression _value;

	/** The decorators of the variable (ref, const ...) */
	std::vector <DecoratorWord> _decos;

    private :

	friend Expression;

	VarDecl ();
	
	VarDecl (const lexing::Word & loc);

    public :

	/**
	 * \brief Create a new VarDecl
	 * \param name the name of the variable
	 * \param decos the decorator of the variable
	 * \param type the type of the var (may be empty ())
	 * \param value the value of the var (may be empty ())
	 */
	static Expression init (const lexing::Word & name, const std::vector <DecoratorWord> & decos, const Expression & type, const Expression & value);
	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;
	
	void setValue (const Expression & value);

	/**
	 * \return the name and location of the declaration
	 */
	const lexing::Word & getName () const;

	/**
	 * \return the type of the declaration
	 */
	const Expression & getType () const;

	/**
	 * \return the value of the declaration
	 */
	const Expression & getValue () const;

	/**
	 * \return true iif the decorator is present in the var declaration
	 */
	bool hasDecorator (Decorator deco) const;

	/**
	 * \return the decorator 
	 */
	const DecoratorWord & getDecorator (Decorator deco) const;

	/**
	 * \return the decorators of the declaration
	 */
	const std::vector <DecoratorWord> & getDecorators () const;
	
    };
    
}
