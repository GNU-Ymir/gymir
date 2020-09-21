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
	
	Set (const lexing::Word & loc, const std::vector <Expression> & params);

    public :

	/**
	 * \brief Create a new Multiple operator
	 * \param params the parameters of the operation
	 */
	static Expression init (const lexing::Word & loc, const std::vector <Expression> & params);
	
	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the content of the set
	 */
	const std::vector <Expression> getContent () const;


	std::string prettyString () const override;
	
    };

}
