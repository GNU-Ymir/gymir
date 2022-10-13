#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	class Integer : public Type {
	public :

	    static std::vector <std::string> NAMES;

	    static int64_t INIT;

	    static std::string INIT_NAME;

	    static std::string MAX_NAME;

	    static std::string MIN_NAME;
	    
	private : 

	    /** The precision of this int type (8/16/32/64/0) */
	    /** 0 means the biggest representable int */
	    int _size; 

	    /** */
	    bool _isSigned;
	    
	private :

	    friend Generator;

	    Integer ();	    

	    Integer (const lexing::Word & loc, int size, bool isSigned = true);
	    
	public :

	    static Generator init (const lexing::Word & loc, int size, bool isSigned = true);

	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;
	    
	    std::string typeName () const override;

	    bool isSigned () const;

	    int getSize () const;
	    
	};
       
    }
}
