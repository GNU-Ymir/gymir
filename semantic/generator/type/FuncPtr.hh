#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	/**
	 * \struct FuncPtr
	 * An array type , this type is always static (we know the size at the compile time)
	 * Dynamic arrays are defined by Slice
	 */
	class FuncPtr : public Type {
	private :	    
	    
	    friend Generator;

	    FuncPtr ();	    

	    FuncPtr (const lexing::Word & loc, const Generator & retType, const std::vector <Generator> & typeParams);
	    
	public :

	    static std::string RET_NAME;

	    static std::string PARAMS_NAME;
	    
	public : 
	    
	    static Generator init (const lexing::Word & loc, const Generator & retType, const std::vector <Generator> & typeParams);

	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;
	    
	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    std::string typeName () const override;

	    
	    /**
	     * \return the return type of the function pointer
	     */
	    const Generator & getReturnType () const;

	    /**
	     * \return a list containing the types of the parameters of the funcPtr
	     */
	    std::vector <Generator> getParamTypes () const;	    

	protected :

	    /**
	     * Does not change the mutability of the params
	     */
	    Generator createMutable (bool is) const override;
	    
	};
       
    }
}
