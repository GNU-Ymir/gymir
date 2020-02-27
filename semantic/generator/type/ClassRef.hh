#pragma once

#include <ymir/semantic/generator/Type.hh>
#include <ymir/semantic/Symbol.hh>

namespace semantic {
    namespace generator {


	/**
	 * \struct ClassRef
	 * \cf Class
	 */
	class ClassRef : public Type {

	    /** The id of the structure refered */
	    std::weak_ptr<ISymbol> _ref;

	    /**
	     * The parent of the class, it can be empty
	     */
	    Generator _parent;

	private :

	    friend Generator;

	    ClassRef ();

	    ClassRef (const lexing::Word & loc, const Generator & parent, const Symbol & ref);

	public :

	    static std::string INIT_NAME;
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & parent, const Symbol & ref);

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

	    Symbol getRef () const;
	    
	    const Generator & getAncestor () const;
	    
	    std::string typeName () const override;

	    std::string getMangledName () const;
	    
	};	
	
    }
}
