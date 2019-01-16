#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Intrinsics
     * Representation of an intrisic
     * \verbatim
     intrin :=  expression '::' elem
                | elem '(' expression ')'
     elem := 'expand' | 'typeof' | 'stringof'
     \endverbatim 
     */
    class Intrinsics : public IExpression {

	/** The location of the allocation */
	lexing::Word _op;

	/** The left operand, can be a type or a value */
	Expression _value;

    private :

	friend Expression;

	Intrinsics ();

    public :

	static Expression init (const Intrinsics & alloc);

	static Expression init (const lexing::Word & op, const Expression & value);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };    

}
