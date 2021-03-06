#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/symbol/ModRef.hh>

namespace semantic {

    namespace generator {

	class ModuleAccess : public Value {

	    std::weak_ptr <ISymbol> _ref;
	    
	private :

	    friend Generator;
	    
	    ModuleAccess ();

	    ModuleAccess (const lexing::Word & loc, const Symbol & ref);

	public :

	    /**
	     */
	    static Generator init (const lexing::Word & loc, const Symbol & ref);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     */
	    std::vector <Symbol> getLocal (const std::string & name) const;

	    /**
	     */
	    std::vector <Symbol> getLocalPublic (const std::string & name) const;

	    /**
	     * \return the module reference that is accessed by this generator
	     */
	    Symbol getModRef () const;

	    std::string prettyString () const override;
	};
	
    }

}
