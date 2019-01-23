#pragma once

#include <ymir/semantic/Generator.hh>

namespace semantic {
    namespace generator {

	class ParamVar : public IGenerator {
	private :

	    /** The type of the generator */
	    Generator _type;

	private :

	    friend Generator;
	    
	    ParamVar ();

	    ParamVar (const lexing::Word & location, const std::string & name, const Generator & type);

	public :

	    /**
	     * \brief Create a param var 
	     * \param location the location of the param var (for debug info)
	     * \param name the name of the param var (unmangled, but with location information included)
	     * \param type the type of the var
	     */
	    static Generator init (const lexing::Word & locatio, const std::string & name, const Generator & type);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    virtual Generator clone () const ;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    virtual bool isOf (const IGenerator * type) const ;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    virtual bool equals (const Generator & other) const ;	    
	    
	    /** 
	     * \return the type of the var
	     */
	    const Generator & getType () const;
	    
	};
	
    }
}
