#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	class Integer : public Type {
	public :

	    static std::vector <std::string> NAMES;

	    static long INIT;
	    
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
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    

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
