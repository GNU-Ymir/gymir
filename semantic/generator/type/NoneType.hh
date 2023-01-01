#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	class NoneType : public Type {

	    std::string _name;
	    
	public :

	    static std::string NAME;
	    
	private :

	    friend Generator;

	    NoneType ();	    

	    NoneType (const lexing::Word & loc, const std::string & name);
	    
	public :

	    static Generator init (const lexing::Word & loc, const std::string & name = "none");

	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    /**
	     * \return the name of the type
	     */
	    std::string typeName () const override;


	    Generator toMutable () const override;

	    Generator toDeeplyMutable () const override;
	    
	protected :

	    Generator createMutable (bool is) const override;
	    
	};
       
    }
}
