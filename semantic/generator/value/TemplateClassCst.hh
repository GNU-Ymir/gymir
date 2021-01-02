#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/semantic/generator/value/TemplateRef.hh>


namespace semantic {

    namespace generator {

	class TemplateClassCst : public TemplateRef {


	    std::vector <syntax::Function::Prototype>  _cst;

	private :

	    friend Generator;

	    TemplateClassCst ();

	    TemplateClassCst (const lexing::Word & loc, const Symbol & ref, const std::vector <syntax::Function::Prototype> & self);

	public :

	    static Generator init (const lexing::Word & loc, const Symbol & ref, const std::vector <syntax::Function::Prototype> & self);

	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * 
	     */
	    const std::vector <syntax::Function::Prototype> & getPrototypes () const;
	    
	    std::string prettyString () const override;

	};
	
    }
    
}
