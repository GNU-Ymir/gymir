#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Binary
	 * A binary generator is an abstract type ancestor of each binary generators	 
	 */
	class Binary : public Value {	    	    
	public : 

	    /**
	     * The list of operators that can generate a simple binary expression 
	     */
	    enum class Operator {
		OR, // ||
		AND, // &&
		INF, // <
		SUP, // >,
		INF_EQUAL, // <=
		SUP_EQUAL, // >=
		NOT_EQUAL, // !=
		EQUAL, // ==
		LEFT_SHIFT, // <<
		RIGHT_SHIFT, // >>
		BIT_OR, // |
		BIT_AND, // &
		BIT_XOR, // ^
		ADD, // +
		CONCAT, // ~
		SUB, // -
		MUL, // *
		DIV, // /
		MODULO, // %
		EXP, // ^^
		RANGE, // ..
		TRANGE, // ...
		LAST_OP
	    };	    

	private : 
	    
	    /** The left operand */
	    Generator _left;

	    /** The right operand */
	    Generator _right;

	    /** The binary operator */
	    Operator _operator;
	    
	protected : 

	    friend Generator;

	    Binary ();

	    Binary (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right);    
	    
	public :

	    static Generator init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /** 
	     * \return the left operand of the operation
	     */
	    const Generator & getLeft () const;

	    /**
	     * \return the right operand of the operation
	     */
	    const Generator & getRight () const;

	    /**
	     * \return the operator of the operation
	     */
	    Operator getOperator () const;
	    
	    std::string prettyString () const override;
	    
	};	

    }
       
}
