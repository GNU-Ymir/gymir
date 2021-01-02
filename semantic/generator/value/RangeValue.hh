#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct RangeValue
	 * RangeValue is an array literal
	 */
	class RangeValue : public Value {

	    /** The left value (start value) */
	    const Generator _left;

	    /** The end value */
	    const Generator _right;

	    /** The step of the range */
	    const Generator _step;

	    /** A bool value defining if the range is full or not (a full range include the end value) */
	    const Generator _isFull;

	private :

	    friend Generator;
	    
	    RangeValue ();

	    RangeValue (const lexing::Word & loc, const Generator & type, const Generator & left, const Generator & right, const Generator & step, const Generator & full);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & left, const Generator & right, const Generator & step, const Generator & full);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the begin value of the range
	     */
	    const Generator & getLeft () const;

	    /**
	     * \return the end value of the range
	     */
	    const Generator & getRight () const;

	    /**
	     * \return the value containing the step of the range 
	     */
	    const Generator & getStep () const;

	    /**
	     * \return the value telling if the range is full or not
	     */
	    const Generator & getIsFull () const;

	    std::string prettyString () const override;
	    
	};
	
    }

}
