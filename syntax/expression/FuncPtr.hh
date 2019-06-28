#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    class FuncPtr : public IExpression {

	/** The parameters of the function */
	std::vector <Expression> _params;

	/** The return type */
	Expression _ret;
	
    private :

	friend Expression;

	FuncPtr ();
	
	FuncPtr (const lexing::Word & loc);

    public :

	/**
	 * \brief Create a new Multiple operator
	 * \param location the location of the operation 
	 * \param end the end of the operation
	 * \param element the left operand
	 * \param params the parameters of the operation
	 */
	static Expression init (const lexing::Word & location, const Expression & ret, const std::vector <Expression> & params);
	
	Expression clone () const override;

	const Expression & getRet () const;

	const std::vector <Expression> & getParams () const;
	
	bool isOf (const IExpression * type) const override;
	
    };

}
