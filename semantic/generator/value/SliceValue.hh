#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class SliceValue : public Value {	    
	private :
	    
	    Generator _ptr;
	    
	    Generator _len;

	protected : 

	    friend Generator;

	    SliceValue ();

	    SliceValue (const lexing::Word & loc, const Generator & type, const Generator & ptr, const Generator & len);	    	    
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & ptr, const Generator & len);	    	    
	    
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
	     * \brief Return the len of the Slice
	     */
	    const Generator & getLen () const;
	    
	    /**
	     * \return the ptr of the slice
	     */
	    const Generator & getPtr () const;
	    
	    std::string prettyString () const override;
	    
	};	
	
    }
       
}

