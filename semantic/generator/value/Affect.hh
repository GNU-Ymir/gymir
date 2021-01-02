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

	    bool _isConstruction;

	private :

	    friend Generator;
	    
	    Affect ();

	    Affect (const lexing::Word & loc, const Generator & type, const Generator & who, const Generator & value, bool isConstruction);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & who, const Generator & value, bool isConstruction = false);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
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

	    bool isConstruction () const;
	    
	    std::string prettyString () const override;
	    
	};
	
    }

}
