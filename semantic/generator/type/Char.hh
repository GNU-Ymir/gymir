#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	class Char : public Type {
	public :

	    static std::vector <std::string> NAMES;

	    /// Value of char are encoded in uint to prevent truncation
	    static uint INIT;
	    
	private : 

	    /** The precision of this float type (8/16/32/0) */
	    /** 0 means the biggest representable char */
	    int _size; 

	private :

	    friend Generator;

	    Char ();	    

	    Char (const lexing::Word & loc, int size);
	    
	public :

	    static Generator init (const lexing::Word & loc, int size);

	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;
	    
	    std::string typeName () const override;

	    int getSize () const;
	    
	};
       
    }
}
