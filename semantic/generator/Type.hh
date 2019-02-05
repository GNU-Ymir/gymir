#pragma once

#include <ymir/semantic/Generator.hh>

namespace semantic {

    namespace generator {

	/**
	 * A type is in opposition of value
	 * A value is typed, but a type has no value
	 */
	class Type : public IGenerator {	    	    
	private :

	    /** This type is a reference to another value of the same type ? */
	    bool _isRef = false;

	    /** This type is a complex one 
	     * - simple affectation result in a implicit referencing, as pointer are contained inside the type
	     * - This is not allowed in the language, so we have to prevent it
	     * - For the moment those types are : 
	     * - - Arrayg
	     */
	    bool _isComplex = false;
	    
	    /** This type contains a value, and is mutable ? */
	    bool _isMutable = false;

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
	    
	    /**
	     * \return does the current type can store the value of the type other without casting ?
	     */
	    virtual bool isCompatible (const Generator & other) const;

	    /**
	     * \return the name of this type formatted
	     */
	    virtual std::string typeName () const ;

	    /**
	     * \return is this type a reference ?
	     */
	    bool isRef () const;
	    /**
	     * \return is this type mutable ?
	     */
	    bool isMutable () const;

	    /**
	     * \return this type is complex ? (cf this-> _isComplex definition)
	     */
	    bool isComplex () const;	    
	    
	    /**
	     * \brief Set if the type is a reference
	     */
	    void isRef (bool is);
	   	    
	    /**
	     * \brief if the type is mutable
	     */
	    void isMutable (bool is);

	protected: 
	    
	    /**
	     * \brief if (is) the type is complex 
	     */
	    void isComplex (bool is);
	    
	};	

    }
       
}
