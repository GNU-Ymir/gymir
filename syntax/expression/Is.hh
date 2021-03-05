#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Is
     */
    class Is : public IExpression {

	/** The element to test */
	Expression _element;

	/** The parameters of the access */
	std::vector <Expression> _params;
	
    private :

	friend Expression;

	Is ();
	
	Is (const lexing::Word & loc, const Expression & element, const std::vector <Expression> & params);

    public :

	/**
	 * \brief Create a new Multiple operator
	 * \param location the location of the operation 
	 * \param end the end of the operation
	 * \param element the left operand
	 * \param params the parameters of the operation
	 */
	static Expression init (const lexing::Word & location, const Expression & element, const std::vector <Expression> & params);
	
	std::string prettyString () const override;

    protected :

	const std::set <std::string> & computeSubVarNames () override;
	
    };

}
