#pragma once

#include <ymir/semantic/Generator.hh>

namespace semantic {
    namespace generator {

	class GlobalVar : public IGenerator {
	private :

	    /** The type of the generator */
	    Generator _type;

	    /** The value of the generator (could be empty ()) */
	    Generator _value;

	    /**
	     * The global var is mutable? 
	     */
	    bool _isMutable;
	    
	private :

	    friend Generator;
	    
	    GlobalVar ();

	    GlobalVar (const lexing::Word & location, const std::string & name, bool isMutable, const Generator & type, const Generator & value);

	public :

	    /**
	     * \brief Create a global var 
	     * \param location the location of the global var (for debug info)
	     * \param name the name of the global var (unmangled, but with location information included)
	     * \param is the global var mutable?
	     * \param type the type of the var
	     * \param value the initial value of the var (could be empty ())
	     */
	    static Generator init (const lexing::Word & locatio, const std::string & name, bool isMutable, const Generator & type, const Generator & value);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    virtual Generator clone () const ;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    virtual bool equals (const Generator & other) const ;	    

	    /**
	     * \return is the variable mutable?
	     */
	    bool isMutable () const;
	    
	    /** 
	     * \return the type of the var
	     */
	    const Generator & getType () const;

	    /**
	     * \return the value of the var
	     */
	    const Generator & getValue () const;
	    
	};
	
    }
}
