#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/symbol/Macro.hh>

namespace semantic {

    namespace generator {

	class MacroRef : public Value {

	    std::weak_ptr<ISymbol> _ref;

	protected :

	    friend Generator;

	    MacroRef ();

	    MacroRef (const lexing::Word & loc, const Symbol & ref);

	public :

	    /**
	     * \brief Generate a new MacroRef access
	     
	     */
	    static Generator init (const lexing::Word & loc, const Symbol & ref);

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

	    /**
	     * \return the return type of the frame prototype
	     */
	    std::vector <Symbol> getLocal (const std::string & name) const;

	    /**
	     * \return the module reference that is accessed by this generator
	     */
	    Symbol getMacroRef () const;

	    std::string prettyString () const override;
	    
	};
	
    }

}
