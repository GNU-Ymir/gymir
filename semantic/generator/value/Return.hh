#pragma once

#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	class Return : public Value {

	    Generator _frameType;
	    
	    Generator _value;

	private :

	    friend Generator;
	    
	    Return ();

	    Return (const lexing::Word & loc, const Generator & type, const Generator & fun_type, const Generator & value);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & fun_type, const Generator & value);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /** 
	     * \return the left operand of the operation
	     */
	    const Generator & getValue () const;

	    /**
	     * \return the type of the function 
	     */
	    const Generator & getFunType () const;
	    
	    std::string prettyString () const override;
	    
	};
	
    }

}
