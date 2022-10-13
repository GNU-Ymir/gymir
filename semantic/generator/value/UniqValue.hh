#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {
    namespace generator {

	class UniqValue : public Value {	    

	    Generator _value;

	    uint32_t _refId;	   

	private :

	    friend Generator;
	    
	    UniqValue ();

	    UniqValue (const lexing::Word & location, const Generator & type, const Generator & value, uint32_t refId);

	public :

	    /**
	     * \brief Create an uniq value, this value is used as a vardecl var ref pair, but in a simpler way
	     * \brief Whenever a value has a risq to be generated multiple time (expand, for loop, ...)
	     * \brief This will ensure that the value is generated only once, and return a ref to it 
	     */
	    static Generator init (const lexing::Word & location, const Generator & type, const Generator & value);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    virtual Generator clone () const ;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    virtual bool equals (const Generator & other) const ;	    

	    /**
	     * \return the initial value of the var
	     */
	    const Generator & getValue () const;

	    /**
	     * \brief Get the ref id of the uniq value
	     */
	    uint32_t getRefId () const;
	    
	    /**
	     * 
	     */
	    std::string prettyString () const override;
	};
       	
    }
}
