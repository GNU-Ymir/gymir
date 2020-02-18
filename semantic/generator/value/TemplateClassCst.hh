#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/syntax/Declaration.hh>
#include <ymir/semantic/generator/value/TemplateRef.hh>


namespace semantic {

    namespace generator {

	class TemplateClassCst : public TemplateRef {


	    syntax::Declaration _cst;

	private :

	    friend Generator;

	    TemplateClassCst ();

	    TemplateClassCst (const lexing::Word & loc, const Symbol & ref, const syntax::Declaration & self);

	public :

	    static Generator init (const lexing::Word & loc, const Symbol & ref, const syntax::Declaration & self);

	    
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

	    const syntax::Declaration & getConstructor () const;
	    
	    std::string prettyString () const override;

	};
	
    }
    
}
