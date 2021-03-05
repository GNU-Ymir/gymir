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

	/** The list of variable declared (iterator) in the for loop */
	std::vector <Expression> _vars;

	/** The expression on which the for loop will iter */
	Expression _iter;

	/** The block content of the for loop */
	Expression _block;

    private :

	friend Expression;

	For ();
	
	For (const lexing::Word & loc, const std::vector <Expression> & vars, const Expression & iter, const Expression & block);

    public :

	/**
	 * \brief Create a new for loop
	 * \param location the position of the loop
	 * \param vars the names of the iterators
	 * \param iter the expression on which the for loop will iter
	 * \param block the content of the loop
	 */
	static Expression init (const lexing::Word & location, const std::vector <Expression> & vars, const Expression & iter, const Expression & block);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getIter () const;

	const Expression & getBlock () const;

	const std::vector <Expression> & getVars () const;

	std::string prettyString () const override;
	
    };
    
}
