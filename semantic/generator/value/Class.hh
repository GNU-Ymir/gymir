#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/generator/type/StructRef.hh>

#include <ymir/semantic/Symbol.hh>

namespace semantic {
    namespace generator {

	class Class : public Value {
	    
	    Symbol _ref;

	    std::vector <generator::Generator> _fields;
	    
	private :

	    friend Generator;

	    friend ClassRef;
	    
	    Class ();

	    Class (const lexing::Word & loc, const Symbol & ref);

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

	    /**
	     * \return the name of the structure (the symbolic one, meaning its space name)
	     */
	    std::string getName () const;

	    /**
	     *
	     */
	    std::string prettyString () const override;

	    /**
	     * \return the symbol responsible of the declaration of this structure prototype
	     */
	    const Symbol&  getRef () const;
	    
	};
    }
    
}
