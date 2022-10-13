#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class CharValue : public Value {	    
	private :
	    
	    uint32_t _value;
	    
	protected : 

	    friend Generator;

	    CharValue ();

	    CharValue (const lexing::Word & loc, const Generator & type, uint32_t value);	    	    
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, uint32_t value);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the value of the float const 
	     */
	    uint32_t getValue () const;

	    std::string prettyString () const override;
	    
	};	
	
    }
       
}

