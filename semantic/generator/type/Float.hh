#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {
    namespace generator {

	class Float : public Type {
	public :

	    static std::vector <std::string> NAMES;

	    /// Value of floating point are encoded in string to prevent approximation
	    static std::string INIT;

	    static std::string NOT_A_NUMBER;

	    static std::string NAN_NAME;

	    static std::string INF_NAME;

	    static std::string INF;

	    static std::string MIN_NAME;

	    static std::string MAX_NAME;

	    static std::string DIG_NAME;

	    static std::string MANT_DIG_NAME;

	    static std::string EPSILON_NAME;

	    static std::string MAX_10_EXP_NAME;

	    static std::string MAX_EXP_NAME;

	    static std::string MIN_10_EXP_NAME;

	    static std::string MIN_EXP_NAME;
	    
	private : 

	    /** The precision of this float type (32/64/0) */
	    /** 0 means the biggest representable float */
	    int _size; 

	private :

	    friend Generator;

	    Float ();	    

	    Float (const lexing::Word & loc, int size);
	    
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
