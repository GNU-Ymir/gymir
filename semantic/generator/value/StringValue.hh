#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class StringValue : public Value {	    
	private :
	    
	    std::vector <char> _value;
	    
	    ulong _len;

	protected : 

	    friend Generator;

	    StringValue ();

	    StringValue (const lexing::Word & loc, const Generator & type, const std::vector<char> & value, ulong len);	    	    
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, const std::vector<char> & value, ulong len);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \brief Return the len of the string
	     */
	    ulong getLen () const;
	    
	    /**
	     * \return the value of the float const 
	     */
	    const std::vector <char> & getValue () const;
	    
	    std::string prettyString () const override;

	    std::string prettyString (bool all) const;
	    
	};	
	
    }
       
}

