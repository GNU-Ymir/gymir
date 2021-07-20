#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct SliceCompare
	 * Compare two slices
	 */
	class SliceCompare : public Value {

	    Generator _value;
	    
	    Generator _left;

	    Generator _right;

	    Binary::Operator _operator;

	private :

	    friend Generator;
	    
	    SliceCompare ();

	    SliceCompare (const lexing::Word & loc, Binary::Operator op, const Generator & type, const Generator & value, const Generator & left, const Generator & right);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, Binary::Operator op, const Generator & type, const Generator & value, const Generator & left, const Generator & right);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    
	    Binary::Operator getOperator () const;
	    
	    /**
	     * @returns the value of the comparison (the one that will be gimplified)
	     */
	    const Generator & getValue () const;
	    
	    /**
	     * \return the left operand
	     */
	    const Generator & getLeft () const;

	    /**
	     * \return the right operand
	     */
	    const Generator & getRight () const;
	    
	    std::string prettyString () const override;
	};
	
    }

}
