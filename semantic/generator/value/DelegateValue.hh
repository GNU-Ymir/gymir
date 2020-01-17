#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class DelegateValue : public Value {	    
	private :

	    Generator _closureType;
	    
	    Generator _closure;
	    
	    Generator _funcptr;

	protected : 

	    friend Generator;

	    DelegateValue ();

	    DelegateValue (const lexing::Word & loc, const Generator & type, const Generator & closureType, const Generator & closure, const Generator & ptr);	    	    
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & closureType, const Generator & closure, const Generator & ptr);	    	    
	    
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
	     * \brief Return the func ptr of the Delegate
	     */
	    const Generator & getFuncPtr () const;
	    
	    /**
	     * \return the closure of the delegate
	     */
	    const Generator & getClosure () const;

	    /**
	     * \return the type of the closure (to cast the closure)
	     */
	    const Generator & getClosureType () const;
	    
	    std::string prettyString () const override;
	    
	};	
	
    }
       
}

