#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Decorator.hh>

namespace syntax {

    /**       
     * \struct RangeType
     * A range type is the syntax encoding of a range type (..i32, ..f32, etc.)
     */
    class RangeType : public IExpression {

	/** The type operand of the range type */ 
	Expression _type;

    private :

	friend Expression;

	RangeType (const lexing::Word & loc, const Expression & inner);

    public :

	/**
	 * Create a new RangeType expression
	 * \param loc the location of the range type
	 * \param type the inner type of the range type expression
	 */
	static Expression init (const lexing::Word & loc, const Expression & type);

	/**
	 * Clone a range type expression
	 */
	static Expression init (const RangeType & other);
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the type inside the range type expression
	 */
	const Expression & getType () const;
	
	std::string prettyString () const override;
	
    };
    

}
