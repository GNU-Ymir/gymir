#pragma once

#include <ymir/semantic/generator/Type.hh>
#include <ymir/semantic/Symbol.hh>

namespace semantic {
    namespace generator {


	/**
	 * \struct StructRef
	 * \cf Struct
	 */
	class StructRef : public Type {

	    /** The id of the structure refered */
	    Symbol _ref;

	private :

	    friend Generator;

	    StructRef ();

	    StructRef (const lexing::Word & loc, const Symbol & ref);
	public :

	    static std::string INIT_NAME;
	    
	public :

	    static Generator init (const lexing::Word & loc, const Symbol & ref);

	    Generator clone () const override;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    bool isRefOf (const Symbol & sym) const;

	    const Symbol & getRef () const;

	    virtual int mutabilityLevel (int level) const override;

	    /**
	     * \return true if this struct contains type with explicit alias needed	     
	     */
	    bool needExplicitAlias () const override;

	    const Generator & getExplicitAliasTypeLoc () const;
	    
	    std::string typeName () const override;

	    std::string getMangledName () const;
	    
	};	
	
    }
}
