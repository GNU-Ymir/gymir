#pragma once

#include <ymir/semantic/Generator.hh>

namespace semantic {

    namespace generator {

	/**
	 * A type is in opposition of value
	 * A value is typed, but a type has no value
	 */
	class Type : public IGenerator {	    	    
	protected :

	    friend Generator;

	    Type ();

	    Type (const lexing::Word & loc, const std::string & name);
	    
	public : 

	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const override ;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;
	    
	    virtual std::string typeName () const ;
	    
	};	

    }
       
}
