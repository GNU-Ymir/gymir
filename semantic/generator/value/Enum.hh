#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/generator/type/EnumRef.hh>

#include <ymir/semantic/Symbol.hh>

namespace semantic {
    namespace generator {

	/**
	 * \struct Enum
	 * The EnumRef is a type, and can be used as a normal structure, at either validation and generation time
	 */
	class Enum : public Value {

	    generator::Generator _type;
	    
	    std::weak_ptr<ISymbol> _ref;
	    
	    std::vector <generator::Generator> _fields;
	    
	private :

	    friend Generator;

	    friend EnumRef;
	    
	    Enum ();

	    Enum (const lexing::Word & loc, const Symbol & ref);

	public : 
	    
	    static Generator init (const lexing::Word & loc, const Symbol & ref);

	    static Generator init (const Enum & en, const Generator & type, const std::vector <generator::Generator> & fields);
	    
	    Generator clone () const override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    /**
	     * \return the fields of the structure
	     */
	    const std::vector <generator::Generator> & getFields () const;

	    /**
	     * \return the type of the enum
	     */
	    Generator getType () const;	    

	    /**
	     * \return the value of the field named name in the enum
	     */
	    const Generator getFieldValue (const std::string & name) const;
	    
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
	    Symbol getRef () const;
	    
	};
    }
    
}
