#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct TupleDest
     * Representation of a tuple destruction
     * \verbatim
     tuple_dest := 'let' '(' var_decl (',' var_decl)* (...)? ')' '=' expression ';'
     var_decl := ('ref')? ('const')? Identifier (':' expression)?
     \endverbatim
     */

    class TupleDest : public IExpression {

	/** The list of the var decl (basically VarDecl)*/
	std::vector <Expression> _vars;

	/** The value of this expression (right operand) */
	Expression _value;

	/** Is this tuple destructor variadic ? (finish with '...') */
	bool _isVariadic;
	
    private :

	friend Expression;

	TupleDest ();
	
	TupleDest (const lexing::Word & loc);

    public :

	/**
	 * \brief Create a new instance of TupleDest
	 * \param location the location of the expression
	 * \param vars the variables of the destruction
	 * \param value the value (right operand)
	 */
	static Expression init (const lexing::Word & location, const std::vector <Expression> & vars, const Expression & value, bool isVariadic = false);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };        

}
