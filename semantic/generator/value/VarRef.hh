#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {
    namespace generator {

	class VarRef : public Value {	    

	    std::string _name;

	    /** The id of the generator that will produce the declaration */
	    uint _refId;

	    /**
	     * is it a reference to a mutable variable ?
	     */
	    bool _isMutable;

	    /** If the this refer to a const var, it could have a compile time value */
	    Generator _value;
	    
	private :

	    friend Generator;
	    
	    VarRef ();

	    VarRef (const lexing::Word & location, const std::string & name, const Generator & type, uint refId, bool isMutable, const Generator & value);

	public :

	    /**
	     * \brief Create a ref var 
	     * \param location the location of the ref var (for debug info)
	     * \param name the name of the ref var (unmangled, but with location information included)
	     * \param type the type of the var
	     * \param refId the id of the generator that will declare the var
	     */
	    static Generator init (const lexing::Word & locatio, const std::string & name, const Generator & type, uint refId, bool isMutable, const Generator & value);
	    
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

	    /**
	     * \return is i a reference to a mutable var ?
	     */
	    bool isMutable () const;	    

	    /**
	     * \return the value of the ref (if any, it could be empty ())
	     */
	    const Generator & getValue () const;
	    
	    std::string prettyString () const override;
	};
	
    }
}
