#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct ArrayAlloc
	 * Alloc a specific index inside an array
	 */
	class ArrayAlloc : public Value {

	    Generator _value;

	    /** The size of the inner type */
	    Generator _size;

	    /** The len of the array (if dynamic) */
	    Generator _len;

	    uint64_t _staticLen;

	    bool _isDynamic;

	private :

	    friend Generator;
	    
	    ArrayAlloc ();

	    ArrayAlloc (const lexing::Word & loc, const Generator & type, const Generator & value, const Generator & size, const Generator & len);

	    ArrayAlloc (const lexing::Word & loc, const Generator & type, const Generator & value, const Generator & size, uint64_t len);

	public :

	    /**
	     * \brief Generate a new dynamic ArrayAlloc 
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & value, const Generator & size, const Generator & len);

	    /**
	     * \brief Generate a new static ArrayAlloc 
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & value, const Generator & size, uint64_t len);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    	   

	    /**
	     * \warning crash if it is not static
	     * \return the len of the ArrayAllocation if it is static
	     */
	    uint64_t getStaticLen () const;

	    /**
	     * \return Tell if the allocation is dynamic
	     */
	    bool isDynamic () const;

	    /**
	     * \warning crash if it is not dynamic
	     * \return the dynamic len
	     */
	    const Generator & getDynLen () const;

	    /**
	     * \return the inner type size 
	     * \info can be retreive with the type of this generator, but it is more convinient this way as it will directly return a SizeOf element
	     */
	    const Generator & getInnerTypeSize () const;

	    /**
	     * \return the default value to put in the array
	     */
	    const Generator & getDefaultValue () const;
	    
	    std::string prettyString () const override;
	};
	
    }

}
