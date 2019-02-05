#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	class Array : public Type {

	    Generator _inner;
	    
	    int _size;
	    
	public :
	    
	    static std::string NAME;
	    
	private :	    
	    
	    friend Generator;

	    Array ();	    

	    Array (const lexing::Word & loc, const Generator & innerType, int size);
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & innerType, int size);

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
	     * \return the inner type of the array
	     */
	    const Generator & getInner () const;

	    /**
	     * \return is this a static array type ?
	     */
	    int size () const;

	    /**
	     * \return the array is static ?
	     */
	    bool isStatic () const;

	    
	};
       
    }
}
