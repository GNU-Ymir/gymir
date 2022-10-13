#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct FieldOffset
	 * Access a specific index of a tuple
	 * The index must be knwon at compile time
	 */
	class FieldOffset : public Value {

	    Generator _str;
	    
	    std::string _field;

	private : 

	    friend Generator;

	    FieldOffset ();

	    FieldOffset (const lexing::Word & loc, const Generator & type, const Generator & str, const std::string & field);

	public:

	    /**
	     * \brief Generate a tuple access
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & tuple, const std::string & field);
	    
	    Generator clone () const override;

	    bool equals (const Generator & other) const override;

	    const Generator & getStruct () const;

	    const std::string & getField () const;
	    
	    std::string prettyString () const override;	    
	    
	};

	
	/**
	 * \struct FieldOffset
	 * Access a specific index of a tuple
	 * The index must be knwon at compile time
	 */
	class FieldOffsetIndex : public Value {

	    Generator _str;
	    
	    uint64_t _field;

	private : 

	    friend Generator;

	    FieldOffsetIndex ();

	    FieldOffsetIndex (const lexing::Word & loc, const Generator & type, const Generator & str, uint64_t index);

	public:

	    /**
	     * \brief Generate a tuple access
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & tuple, uint64_t index);
	    
	    Generator clone () const override;

	    bool equals (const Generator & other) const override;

	    const Generator & getTuple () const;

	    uint64_t getField () const;
	    
	    std::string prettyString () const override;	    
	    
	};
	
    }
    
}
