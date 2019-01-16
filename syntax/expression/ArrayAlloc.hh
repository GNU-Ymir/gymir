#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct ArrayAlloc
     * Representation of an array allocation, static or dynamic
     * \verbatim
     array_alloc := '[' expression ';' ('new')? expression ']'
     \endverbatim 
     */
    class ArrayAlloc : public IExpression {

	/** The location of the allocation */
	lexing::Word _location;

	/** The left operand, can be a type or a value */
	Expression _left;

	/** The size of the allocation */
	Expression _size;

	/** Is this array allocated on heap ? ('new' token) */
	bool _isDynamic;


    private :

	friend Expression;

	ArrayAlloc ();

    public :

	static Expression init (const ArrayAlloc & alloc);

	static Expression init (const lexing::Word & location, const Expression & left, const Expression &size, bool isDynamic = false);

	Expression clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	bool isOf (const IExpression * type) const override;
	
    };    

}