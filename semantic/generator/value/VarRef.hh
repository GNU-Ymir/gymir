#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {
    namespace generator {

	class VarRef : public Value {	    

	    std::string _name;

	    /** The id of the generator that will produce the declaration */
	    uint _refId;
	    
	private :

	    friend Generator;
	    
	    VarRef ();

	    VarRef (const lexing::Word & location, const std::string & name, const Generator & type, uint refId);

	public :

	    /**
	     * \brief Create a ref var 
	     * \param location the location of the ref var (for debug info)
	     * \param name the name of the ref var (unmangled, but with location information included)
	     * \param type the type of the var
	     * \param refId the id of the generator that will declare the var
	     */
	    static Generator init (const lexing::Word & locatio, const std::string & name, const Generator & type, uint refId);
	    
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
	     * \return the id of the generator that will produce the var declaration
	     */
	    uint getRefId () const;
	    	    
	};
	
    }
}
