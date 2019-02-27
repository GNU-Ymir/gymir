#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct SliceAccess
	 * Access a specific index inside an array
	 */
	class SliceAccess : public Value {

	    Generator _slice;

	    Generator _index;

	private :

	    friend Generator;
	    
	    SliceAccess ();

	    SliceAccess (const lexing::Word & loc, const Generator & type, const Generator & slice, const Generator & index);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & slice, const Generator & index);
	    
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
	     * \return the generator of the slice that will be accessed
	     */
	    const Generator & getSlice () const;

	    /**
	     * \return the index 
	     */
	    const Generator & getIndex () const;
	    
	};
	
    }

}
