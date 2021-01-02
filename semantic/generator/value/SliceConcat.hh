#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct SliceConcat
	 * Concat two slices
	 */
	class SliceConcat : public Value {

	    Generator _left;

	    Generator _right;

	private :

	    friend Generator;
	    
	    SliceConcat ();

	    SliceConcat (const lexing::Word & loc, const Generator & type, const Generator & left, const Generator & right);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & left, const Generator & right);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the left operand
	     */
	    const Generator & getLeft () const;

	    /**
	     * \return the right operand
	     */
	    const Generator & getRight () const;
	    
	    std::string prettyString () const override;
	};
	
    }

}
