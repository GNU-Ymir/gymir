#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	class Void : public Type {
	public :

	    static std::string NAME;

	private :

	    friend Generator;

	    Void ();	    

	    Void (const lexing::Word & loc);
	    
	public :

	    static Generator init (const lexing::Word & loc);

	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    /**
	     * \return the name of the type
	     */
	    std::string typeName () const override;
	   
	    void setMutable (bool) override;

	    Generator createMutable (bool) const override;

	    Generator toMutable () const override;
	    
	    Generator toDeeplyMutable () const override;
	    
	};
       
    }
}
