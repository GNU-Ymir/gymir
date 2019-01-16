#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>

namespace syntax {

    /**
     * \struct For
     * Representation of a For loop
     * This reprensentation can be a part of a cte expression
     * \verbatim
     for := 'for' vars 'in' expression expression
     vars := var (',' var)*
     var := ('const')? ('ref')? Identifier
     \endverbatim
     */
    class For : public IExpression {

	/** The location of the for loop */
	lexing::Word _location;

	/** The list of variable declared (iterator) in the for loop */
	std::vector <Expression> _vars;

	/** The expression on which the for loop will iter */
	Expression _iter;

	/** The block content of the for loop */
	Expression _block;

    private :

	friend Expression;

	For ();

    public :

	/**
	 * \brief Make a copy of a for 
	 * \param for_ the expression to copy
	 */
	static Expression init (const For & for_);

	/**
	 * \brief Create a new for loop
	 * \param location the position of the loop
	 * \param vars the names of the iterators
	 * \param iter the expression on which the for loop will iter
	 * \param block the content of the loop
	 */
	static Expression init (const lexing::Word & location, const std::vector <Expression> & vars, const Expression & iter, const Expression & block);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };
    
}
