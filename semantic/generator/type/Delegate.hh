#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	/**
	 * \struct Delegate
	 * An array type , this type is always static (we know the size at the compile time)
	 * Dynamic arrays are defined by Slice
	 */
	class Delegate : public Type {
	private :	    
	    
	    friend Generator;

	    Delegate ();	    

	    Delegate (const lexing::Word & loc, const Generator & funcPtr);
	    
	public :

	    static std::string RET_NAME;

	    static std::string PARAMS_NAME;
	    
	public : 
	    
	    static Generator init (const lexing::Word & loc, const Generator & funcPtr);

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
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    std::string typeName () const override;

	protected :
	    
	    /**
	     * Does not change the mutability of the params
	     */
	    Generator createMutable (bool is) const override;
	    
	};
       
    }
}
