#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/generator/type/StructRef.hh>

#include <ymir/semantic/Symbol.hh>

namespace semantic {
    namespace generator {

	/**
	 * \struct Struct
	 * A structure stores the first level of its fields to prevent cycle
	 * \verbatim
	 struct 
	 | a : &Z // ptr of Z
	 -> Z; 
	 \endverbatim

	 * Structure class will have inner types as follow : 
	 \verbatim
	 Struct (Z) : a -> Pointer (StructRef (Z))	 
	 \endverbatim
	 The StructRef is a type, and can be used as a normal structure, at either validation and generation time
	 */
	class Struct : public Value {
	    
	    std::weak_ptr <ISymbol> _ref;

	    std::vector <generator::Generator> _fields;
	    
	private :

	    friend Generator;

	    friend StructRef;
	    
	    Struct ();

	    Struct (const lexing::Word & loc, const Symbol & ref);

	public : 
	    
	    static Generator init (const lexing::Word & loc, const Symbol & ref);

	    static Generator init (const Struct & other, const std::vector <Generator> & fields);
	    
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
	     * \return the type of a given field or an empty generator 	     
	     */
	    Generator  getFieldType (const std::string & name) const;

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

	    /**
	     * \return does this structure have at least one complex field ? 
	     */
	    bool hasComplexField () const;
	    
	};
    }
    
}
