#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct A slice is an array with a dynamic size
	 * It can borrow data from static array, are allocated on the heap
	 */
	class Slice : public Type {

	    /** If this-> _inner.isMutable () == true, it means that we can modify the values of each index */
	    /** It can be represented as the following type : [mut T] and means : */
	    /** 
		let mut a : [mut i32] = [1, 2, 3]; 
		a [0] = 9; // Ok
		a = [2, 4, 5]; // Ok

		let mut b : [char] = "test"; 
		b [0] = 't'; // No
		b = "foo"; // Ok
	     */
	    
	private : 

	    friend Generator;

	    Slice ();

	    Slice (const lexing::Word & loc, const Generator & innerType);

	public : 

	    static std::string LEN_NAME;
	    
	    static std::string PTR_NAME;

	    static std::string INNER_NAME;

	    static std::string INIT;
	    
	public : 

	    static Generator init (const lexing::Word & loc, const Generator & innerType);
	    
	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;


	    /**
	     * \return true, the data of a slice are in the heap or in a place that is not copied when simple affectation is made
	     */
	    bool needExplicitAlias () const override;
	    
	    /**
	     * \return is this type compatible with other (assuming other is a type)
	     */
	    bool isCompatible (const Generator & other) const override;

	    /** 
	     * \return the name of the type formatted
	     */
	    std::string typeName () const override;

	    
	};

    }
    
}
