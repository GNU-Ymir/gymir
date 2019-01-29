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

	    /**
	     * this value is set to true if the current generator break the closest loop scope in any case
	     */
	    bool _breaker = false;

	    /**
	     * this value is set to true if the current generator close the frame in any case
	     */
	    bool _returner = false;
	    
	protected : 

	    friend Generator;

	    Value ();

	    Value (const lexing::Word & loc, const Generator & type);

	    Value (const lexing::Word & loc, const std::string & name, const Generator & type);
	    
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
	     * \brief change the type of this value
	     */
	    void setType (const Generator & type);
	    
	    /** 
	     * \return the type of the value
	     */
	    const Generator & getType () const;

	    /**
	     * \return Tell if the generator close the closest loop scope
	     */
	    bool isBreaker () const;

	    /**
	     * \brief Change the state of this value
	     * \param breaker if true reaching this value close the closest loop scope  
	     */
	    void isBreaker (bool breaker);
	    
	    /**
	     * \return Does this generator close the frame ?
	     */
	    bool isReturner () const;

	    /**
	     * \brief Change the state of this value
	     * \param returnable if true reaching this value close the current frame
	     */
	    void isReturner (bool returner);
	    
	};	

    }
       
}
