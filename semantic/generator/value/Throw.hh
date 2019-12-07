#pragma once

#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {


	class Throw : public Value {
	    
	    Generator _value;

	private :

	    friend Generator;
	    
	    Throw ();

	    Throw (const lexing::Word & loc, const Generator & value);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     */
	    static Generator init (const lexing::Word & loc, const Generator & value);
	    
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
	     * \return the value that will be thrown
	     */
	    const Generator & getValue () const;
	    
	    std::string prettyString () const override;	    
	    
	};
	
    }

}
