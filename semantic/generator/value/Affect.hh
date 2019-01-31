#pragma once

#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Binary 
	 * Binary operations on ints
	 */
	class Affect : public Value {

	    Generator _who;

	    Generator _value;	    

	private :

	    friend Generator;
	    
	    Affect ();

	    Affect (const lexing::Word & loc, const Generator & type, const Generator & who, const Generator & value);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & who, const Generator & value);
	    
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
	    const Generator & getWho () const;

	    /**
	     * \return the right operand of the operation
	     */
	    const Generator & getValue () const;

	};
	
    }

}
