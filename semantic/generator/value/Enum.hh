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
	    
	    Symbol _ref;

	    generator::Generator _type;
	    
	    std::vector <generator::Generator> _fields;
	    
	private :

	    friend Generator;

	    friend EnumRef;
	    
	    Enum ();

	    Enum (const lexing::Word & loc, const Symbol & ref);

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
	     * \return the fields of the structure
	     */
	    const std::vector <generator::Generator> & getFields () const;

	    /**
	     * \return the type of the enum
	     */
	    Generator getType () const;

	    /**
	     * \brief Set the type of the generator
	     */
	    void setType (const generator::Generator & type);
	    
	    /**
	     * set the fields of the struct
	     */
	    void setFields (const std::vector <generator::Generator> & fields);

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
	    const Symbol&  getRef () const;
	    
	};
    }
    
}
