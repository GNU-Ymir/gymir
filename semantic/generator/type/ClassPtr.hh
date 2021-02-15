#pragma once

#include <ymir/semantic/generator/Type.hh>
#include <ymir/semantic/generator/type/ClassRef.hh>

namespace semantic {

    namespace generator {

	class ClassPtr : public Type {
	protected :

	    Generator _ref;
	    
	protected : 

	    friend Generator;

	    ClassPtr ();

	    ClassPtr (const lexing::Word & loc, const Generator & innerType);

	public : 


	    static Generator init (const lexing::Word & loc, const Generator & innerType);
	    
	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    /**
	     * \return is this type compatible with other (assuming other is a type)
	     */
	    bool isCompatible (const Generator & other) const override;

	    /**
	     * \return true, pointer points to data in foreign place, and need to give the ownership
	     */
	    bool needExplicitAlias () const override;

	    /**
	     * \return true, this is literaly a pointer
	     */
	    bool containPointers () const override;
	    
	    /**
	     * \return the associated class reference
	     */
	    const ClassRef & getClassRef () const;
	    
	    /** 
	     * \return the name of the type formatted
	     */
	    std::string typeName () const override;

	    
	};

    }
    
}
