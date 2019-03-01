#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class BoolValue : public Value {	    
	private :
	    
	    bool _value;
	    
	protected : 

	    friend Generator;

	    BoolValue ();

	    BoolValue (const lexing::Word & loc, const Generator & type, bool value);	    	    
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, bool value);
	    
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
	     * \return the value of the fixed const, can be signed or unsigned depending on the type
	     */
	    bool getValue () const;

	    std::string prettyString () const override;
	    
	};	
	
    }
       
}

