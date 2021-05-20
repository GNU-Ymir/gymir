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
	    
	    ulong _field;

	private : 

	    friend Generator;

	    FieldOffsetIndex ();

	    FieldOffsetIndex (const lexing::Word & loc, const Generator & type, const Generator & str, ulong index);

	public:

	    /**
	     * \brief Generate a tuple access
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & tuple, ulong index);
	    
	    Generator clone () const override;

	    bool equals (const Generator & other) const override;

	    const Generator & getTuple () const;

	    ulong getField () const;
	    
	    std::string prettyString () const override;	    
	    
	};
	
    }
    
}
