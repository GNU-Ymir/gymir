#pragma once

#include <ymir/semantic/generator/type/ClassPtr.hh>
#include <ymir/semantic/generator/type/ClassRef.hh>

namespace semantic {

    namespace generator {

	/**
	 * A proxy class is used to call the method of a parent class
	 * They are invoked by self::#{parentName}
	 */
	class ClassProxy : public ClassPtr {
	private :

	    Generator _proxy;
	    
	private : 

	    friend Generator;

	    ClassProxy ();

	    ClassProxy (const lexing::Word & loc, const Generator & type, const Generator & ancestor);

	public : 


	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & ancestor);
	    
	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (not only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    /**
	     * \return the ref that generated the proxy
	     */
	    const ClassRef & getProxyRef () const;
   	    
	    /** 
	     * \return the name of the value formatted
	     */
	    std::string typeName () const override;

	    
	};

    }
    
}
