#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct A slice is an array with a dynamic size
	 * It can borrow data from static array, are allocated on the heap
	 */
	class Tuple : public Type {	    
	public :

	    const static std::string ARITY_NAME;

	    const static std::string INIT_NAME;
	    
	private : 

	    friend Generator;

	    Tuple ();

	    Tuple (const lexing::Word & loc, const std::vector <Generator> & innerType);

	public : 


	    static Generator init (const lexing::Word & loc, const std::vector <Generator> & innerType);
	    
	    Generator clone () const override;

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
