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

	/** The names of the iterators */
	std::vector <lexing::Word> _names;

	/** The decorators of the iterators */
	std::vector <std::vector <Decorator> > _decos;

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
	 * \param names the names of the iterators
	 * \param decos the decorators of the iterators (const, ref...)
	 * \param iter the expression on which the for loop will iter
	 * \param block the content of the loop
	 */
	static Expression init (const lexing::Word & location, const std::vector <lexing::Word> & names, const std::vector <std::vector <Decorator> > & decos, const Expression & iter, const Expression & block);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };
    
}
