#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct VtableAccess
	 * Access a specific index of a tuple
	 * The index must be knwon at compile time
	 */
	class VtableAccess : public Value {

	    Generator _str;
	    
	    uint32_t _field;

	    std::string _name;

	    bool _uniq;

	private : 

	    friend Generator;

	    VtableAccess ();

	    VtableAccess (const lexing::Word & loc, const Generator & type, const Generator & cl, uint32_t field, const std::string & name);

	public:

	    /**
	     * \brief Generate a tuple access
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & cl, uint32_t field, const std::string & name);
	    
	    Generator clone () const override;


	    bool equals (const Generator & other) const override;

	    const Generator & getClass () const;

	    uint32_t getField () const;
	    
	    std::string prettyString () const override;	    
	    
	};
	
    }
    
}
