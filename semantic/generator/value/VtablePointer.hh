#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * Get the vtable pointer for a given class type
	 */
	class VtablePointer : public Value {

	    Generator _str;	   

	private : 

	    friend Generator;

	    VtablePointer ();

	    VtablePointer (const lexing::Word & loc, const Generator & type, const Generator & cl);

	public:

	    /**
	     * \brief Generate a tuple access
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & cl);
	    
	    Generator clone () const override;

	    bool equals (const Generator & other) const override;

	    const Generator & getClass () const;
	    
	    std::string prettyString () const override;	    
	    
	};
	
    }
    
}
