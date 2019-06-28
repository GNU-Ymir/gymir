#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	class NoneType : public Type {
	public :

	    static std::string NAME;

	private :

	    friend Generator;

	    NoneType ();	    

	    NoneType (const lexing::Word & loc);
	    
	public :

	    static Generator init (const lexing::Word & loc);

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
	     * \return the name of the type
	     */
	    std::string typeName () const override;
	    
	};
       
    }
}
