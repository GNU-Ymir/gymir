#pragma once

#include <ymir/semantic/generator/Type.hh>
#include <ymir/semantic/Symbol.hh>

namespace semantic {
    namespace generator {


	/**
	 * \struct TraitRef
	 * \cf Trait
	 */
	class TraitRef : public Type {

	    /** The id of the structure refered */
	    Symbol _ref;


	private :

	    friend Generator;

	    TraitRef ();

	    TraitRef (const lexing::Word & loc, const Symbol & ref);

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

	    bool needExplicitAlias () const override;

	    const Symbol & getRef () const;
	    
	    std::string typeName () const override;

	    std::string getMangledName () const;
	    
	};	
	
    }
}
