#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct List
     * A list, is a sequence of expression not surrounded
     * It is used to list a set of expression written in a single line 
     * Same as block in some terms, but does not refer to a scope
     * Do not confuse with List
     */
    class Set : public IExpression {

	/** The parameters of the access */
	std::vector <Expression> _params;
	
    private :

	friend Expression;

	Set ();

    public :

	/**
	 * \brief Make a copy of a mult operator
	 * \param op the operator to copy
	 */
	static Expression init (const Set & op);

	/**
	 * \brief Create a new Multiple operator
	 * \param params the parameters of the operation
	 */
	static Expression init (const std::vector <Expression> & params);
	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };

}
