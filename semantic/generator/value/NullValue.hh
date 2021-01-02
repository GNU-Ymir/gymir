#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class NullValue : public Value {	    
	protected : 

	    friend Generator;

	    NullValue ();

	    NullValue (const lexing::Word & loc, const Generator & type);	    	    
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    


	    std::string prettyString () const override;
	    
	};	
	
    }
       
}

