#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class None : public Value {	    	    	   	    

	    friend Generator;

	    None ();

	    None (const lexing::Word & loc);   
	    
	public :

	    static Generator init (const lexing::Word & loc);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    
	    
	};	

    }
       
}
