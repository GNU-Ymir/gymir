#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	/**
	 * \struct Array
	 * An array type , this type is always static (we know the size at the compile time)
	 * Dynamic arrays are defined by Slice
	 */
	class Array : public Type {
	    
	    uint _size;	   
	    
	private :	    
	    
	    friend Generator;

	    Array ();	    

	    Array (const lexing::Word & loc, const Generator & innerType, uint size);
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & innerType, uint size);

	    Generator clone () const override;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    std::string typeName () const override;
	    
	    /**
	     * \return is this a static array type ?
	     */
	    uint getSize () const;
	    
	};
       
    }
}
