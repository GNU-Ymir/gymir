#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/generator/value/TemplateRef.hh>


namespace semantic {

    namespace generator {

	class MethodTemplateRef : public TemplateRef {


	    Generator _self;

	private :

	    friend Generator;

	    MethodTemplateRef ();

	    MethodTemplateRef (const lexing::Word & loc, const Symbol & ref, const Generator & self);

	public :

	    static Generator init (const lexing::Word & loc, const Symbol & ref, const Generator & self);

	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    const Generator & getSelf () const;
	    
	    std::string prettyString () const override;

	};
	
    }
    
}
