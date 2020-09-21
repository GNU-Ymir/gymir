#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>

namespace syntax {

    /**
     * \struct ArrayAlloc
     * Representation of an array allocation, static or dynamic
     * \verbatim
     array_alloc := '[' expression ';' ('new')? expression ']'
     \endverbatim 
     */
    class ArrayAlloc : public IExpression {

	/** The left operand, can be a type or a value */
	Expression _left;

	/** The size of the allocation */
	Expression _size;

	/** Is this array allocated on heap ? ('new' token) */
	bool _isDynamic;

    private :

	friend Expression;

	ArrayAlloc ();

	ArrayAlloc (const lexing::Word & loc, const Expression & left, const Expression & size, bool isDyn);

    public :

	static Expression init (const lexing::Word & location, const Expression & left, const Expression &size, bool isDynamic = false);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	bool isOf (const IExpression * type) const override;

	/**
	 * \return the left part of the allocation (could be a type or any expression)
	 */
	const Expression & getLeft () const;

	/**
	 * \return the size part of the allocation
	 */
	const Expression & getSize () const;

	/**
	 * \return is this allocation dynamic (declared with the new token)
	 */
	bool isDynamic () const;

	std::string prettyString () const override;
	
    };    

}
