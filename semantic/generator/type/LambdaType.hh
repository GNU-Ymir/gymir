#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct A slice is an array with a dynamic size
	 * It can borrow data from static array, are allocated on the heap
	 */
	class LambdaType : public Type {
	public :

	    static std::string ARITY_NAME;
	    
	private : 

	    friend Generator;

	    LambdaType ();

	    LambdaType (const lexing::Word & loc, const Generator & retType, const std::vector <Generator> & innerType);

	public : 


	    static Generator init (const lexing::Word & loc, const Generator & retType, const std::vector <Generator> & innerType);
	    
	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	   
	    
	    /** 
	     * \return the name of the type formatted
	     */
	    std::string typeName () const override;

	protected :

	    Generator createMutable (bool is) const override;
	    
	};

    }
    
}
