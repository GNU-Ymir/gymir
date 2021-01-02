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

	/** The left operand, can be a type or a value */
	Expression _value;

    private :

	friend Expression;

	Intrinsics ();
	
	Intrinsics (const lexing::Word & loc, const Expression & value);

    public :

	static Expression init (const lexing::Word & op, const Expression & value);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the value on which this intrinsics is applied
	 */
	const Expression & getContent () const;

	/**
	 * \return is it a copy ?
	 */
	bool isCopy () const;

	/**
	 * \return is it an alias ?
	 */
	bool isAlias () const;

	/**
	 * \return is it an expand ?
	 */
	bool isExpand () const;

	/**
	 * \return is it a typeof ?
	 */
	bool isTypeof () const;

	/**
	 * \return is it a sizeof ?
	 */
	bool isSizeof () const;

	/**
	 * \return is it a deep copy (dcopy) ?
	 */
	bool isDeepCopy () const;


	/**
	 * \return is it a move
	 */
	bool isMove () const;

	std::string prettyString () const override;
	
    };    

}
