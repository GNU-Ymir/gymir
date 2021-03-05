#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	class Bool : public Type {
	public :
	    
	    static std::string NAME;

	    static bool INIT;
	    
	private :	    
	    
	    friend Generator;

	    Bool ();	    

	    Bool (const lexing::Word & loc);
	    
	public :

	    static Generator init (const lexing::Word & loc);

	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    
	    std::string typeName () const override;
	    
	};
       
    }
}
