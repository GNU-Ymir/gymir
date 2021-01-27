#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A throw block is an optimization 
	 * It is a part of code in a frame that throws an exception
	 * Each time the frame is going to throw an exception, (must be the exactly the same one, such as outofarray, seg fault...)
	 * Instead of calling the function that will throw, they goto this segment that will throw
	 * The idea is that by having a uniq function call, GCC will be able to make more optimization
	 * \example: 
	 * Ymir: 
	 * =============
	 * a[0] = 1;
	 * a[1] = 2;
	 * =============
	 * =============	 
	 * frame: 
	 *     if (0 >= a.len) {
	 *         goto throw;
	 *     } else a [0] = 1; 
	 *     if (1 >= a.len) {
	 *         goto throw;
	 *     } else a [1] = 2; 
	 * throw_bl:
	 *    goto end;
	 * throw: 
	 *    call core::array::outOfArray;
	 * end:
	 * =============
	 * 
	 * What is expected as optimization : 
	 * =============
	 * if (a.len <= 1) goto throw;
	 * a [0] = 1;
	 * a [1] = 2;
	 * throw_bl: 
	 *    goto end
	 * throw: 
	 *    call core::array::outOfArray;
	 * end:
	 * =============
	 */
	class ThrowBlock : public Value {
	    
	    Generator _content;

	    std::string _name;
	    
	private: 
	    
	    ThrowBlock (const lexing::Word & loc, const Generator & content, const std::string & name);

	public:
	    
	    static Generator init (const lexing::Word & loc, const Generator & content, const std::string & name);

	    /**
	     * The block to execute when entering a throw block
	     */
	    const Generator & getContent () const;

	    /**
	     * The uniq name, that assert that every throw block with the same name in a frame will do the same thing
	     */
	    const std::string & getName () const;

	    
	    std::string prettyString () const override;
	    
	};
	
    }
    
}
