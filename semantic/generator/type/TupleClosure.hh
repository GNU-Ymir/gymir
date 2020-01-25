#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct A slice is an array with a dynamic size
	 * It can borrow data from static array, are allocated on the heap
	 */
	class TupleClosure : public Type {	    	    
	private : 

	    friend Generator;

	    TupleClosure ();

	    TupleClosure (const lexing::Word & loc, const std::vector <Generator> & innerType);

	public : 


	    static Generator init (const lexing::Word & loc, const std::vector <Generator> & innerType);
	    
	    Generator clone () const override;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    /**
	     * \return true if this tuple contains type with explicit alias needed	     
	     */
	    bool needExplicitAlias () const override;

	    /** 
	     * \return the name of the type formatted
	     */
	    std::string typeName () const override;
	    
	};

    }
    
}
