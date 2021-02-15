#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	/**
	 * \struct Range
	 * An array type , this type is always static (we know the size at the compile time)
	 * Dynamic arrays are defined by Slice
	 */
	class Range : public Type {	    	    
	private :	    
	    
	    friend Generator;

	    Range ();	    

	    Range (const lexing::Word & loc, const Generator & innerType);
	    
	public :	    

	    static std::string NAME;
	    
	    static std::string INNER_NAME;

	    static std::string FST_NAME;

	    static std::string SCD_NAME;

	    static std::string STEP_NAME;

	    static std::string FULL_NAME;
	    
	public : 
	    
	    static Generator init (const lexing::Word & loc, const Generator & innerType);

	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool directEquals (const Generator & other) const override;

	    /**
	     * \return the type of the step element
	     * \brief signed fixed if the inner type is an integer, or a char
	     * \brief float otherwise
	     */
	    Generator getStepType () const; 
	    
	    std::string typeName () const override;
	    
	};
       
    }
}

