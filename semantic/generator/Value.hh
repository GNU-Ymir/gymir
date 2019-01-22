#pragma once

#include <ymir/semantic/Generator.hh>
#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	/**
	 * A value is in opposition of type
	 * A value is typed, but a type has no value
	 */
	class Value : public IGenerator {	    	    
	private :

	    Generator _type;
	    
	protected : 

	    friend Generator;

	    Value ();

	    Value (const Generator & type);	    	    
	    
	public :

	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    virtual Generator clone () const ;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    virtual bool isOf (const IGenerator * type) const ;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    virtual bool equals (const Generator & other) const ;	    
	    
	    /** 
	     * \return the type of the value
	     */
	    const Generator & getType () const;
	    
	};	

    }
       
}
