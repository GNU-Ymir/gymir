#pragma once

#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {


	class Panic : public Value {	    	    
	private :

	    friend Generator;
	    
	    Panic ();

	    Panic (const lexing::Word & loc);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     */
	    static Generator init (const lexing::Word & loc);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    
	    
	    std::string prettyString () const override;	    
	    
	};
	
    }

}
