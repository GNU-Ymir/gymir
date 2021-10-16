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
	    
	    uint _field;

	    std::string _name;

	    bool _uniq;

	private : 

	    friend Generator;

	    VtableAccess ();

	    VtableAccess (const lexing::Word & loc, const Generator & type, const Generator & cl, uint field, const std::string & name);

	public:

	    /**
	     * \brief Generate a tuple access
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & cl, uint field, const std::string & name);
	    
	    Generator clone () const override;


	    bool equals (const Generator & other) const override;

	    const Generator & getClass () const;

	    uint getField () const;
	    
	    std::string prettyString () const override;	    
	    
	};
	
    }
    
}
