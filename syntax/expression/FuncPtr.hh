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
	
	FuncPtr (const lexing::Word & loc, const Expression & ret, const std::vector <Expression> & params);

    public :

	/**
	 * \brief Create a new Multiple operator
	 * \param location the location of the operation 
	 * \param end the end of the operation
	 * \param element the left operand
	 * \param params the parameters of the operation
	 */
	static Expression init (const lexing::Word & location, const Expression & ret, const std::vector <Expression> & params);       

	const Expression & getRetType () const;

	const std::vector <Expression> & getParameters () const;
	
	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;
	
    };

}
