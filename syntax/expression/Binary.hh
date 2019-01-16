#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Binary
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Binary : public IExpression {

	/** The location of the allocation */
	lexing::Word _op;

	/** The left operand, can be a type or a value */
	Expression _left;

	/** The size of the allocation */
	Expression _right;

	/** Is this array allocated on heap ? ('new' token) */
	Expression _type;

    private :

	friend Expression;

	Binary ();

    public :

	static Expression init (const Binary & alloc);

	static Expression init (const lexing::Word & op, const Expression & left, const Expression &right, const Expression & type);

	Expression clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
