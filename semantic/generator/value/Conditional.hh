#pragma once

#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Binary 
	 * Binary operations on ints
	 */
	class Conditional : public Value {

	    Generator _test;

	    Generator _content;

	    Generator _else;

	private :

	    friend Generator;
	    
	    Conditional ();

	    Conditional (const lexing::Word & loc, const Generator & type, const Generator & test, const Generator & content, const Generator & else_);

	public :

	    /**
	     * \brief Generate a 
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & test, const Generator & content, const Generator & else_);
	    
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
	     * \return the test of the condition
	     */
	    const Generator & getTest () const;

	    /**
	     * \return the content to execute if the test if valid
	     */
	    const Generator & getContent () const;

	    /**
	     * \return the content to execute if the test is not valid
	     */
	    const Generator & getElse () const;
	    
	};
	
    }

}
