#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Return
     * Representation of a return expression 
     * \verbatim
     return := 'return' (expression)? ';'
     \endverbatim
     */
    class Return : public IExpression {

	/** The value to return */
	Expression _value;

    private :

	friend Expression;

	Return ();
	
	Return (const lexing::Word & loc, const Expression & value);

    public :

	/**
	 * \brief Create a new return expression 
	 * \param location the location of the expression
	 * \param value the value to return (may be empty ())
	 */
	static Expression init (const lexing::Word & location, const Expression & value);

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getValue () const;

	std::string prettyString () const override;
	
    };    
    
}
