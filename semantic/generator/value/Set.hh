#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class Set : public Value {	    	    
	    
	    std::vector <Generator> _content;
	    
	protected : 

	    friend Generator;

	    Set ();

	    Set (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content);	    	    
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & values);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the list of expression defined in the block
	     */
	    const std::vector <Generator> & getContent () const;


	    std::string prettyString () const override;
	    
	};	

    }
       
}
