#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Unary
	 * A binary generator is an abstract type ancestor of each binary generators	 
	 */
	class Unary : public Value {	    	    
	public : 

	    /**
	     * The list of operators that can generate a simple unary expression 
	     */
	    enum class Operator {
		NOT , // !
		MINUS, // - 
		UNREF, // *
		ADDR, // &
		LAST_OP
	    };	    

	private : 
	    
	    /** The operand */
	    Generator _operand;

	    /** The binary operator */
	    Operator _operator;
	    
	protected : 

	    friend Generator;

	    Unary ();

	    Unary (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand);    
	    
	public :

	    static Generator init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand);
	    
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
	     * \return the operand of the operation
	     */
	    const Generator & getOperand () const;

	    /**
	     * \return the operator of the operation
	     */
	    Operator getOperator () const;
	    
	};	

    }
       
}
