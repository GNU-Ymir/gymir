#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A Lblock just generate a block, but as a lvalue
	 *  Left block return value that has been declared in the above scope
	 * They are treated differentely at generation time, and are lvalue
	 */
	class LBlock {	    	    	    	    
	public : 
	    static Generator init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & values);
	};	

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class Block : public Value {	    	    
	    
	    std::vector <Generator> _content;
	    
	protected : 

	    friend Generator;
	    friend LBlock;

	    Block ();

	    Block (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content, bool isLvalue);	    	    	    
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & values);
	    
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
	     * \return the list of expression defined in the block
	     */
	    const std::vector <Generator> & getContent () const;

	    /**
	     * \brief Set the list of expression of the block
	     */
	    void setContent (const std::vector <Generator> & expressions);

	    std::string prettyString () const override;
	};	

	
    }
       
}
