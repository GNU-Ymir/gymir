#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct TupleAccess
	 * Access a specific index of a tuple
	 * The index must be knwon at compile time
	 */
	class TupleAccess : public Value {

	    Generator _tuple;
	    
	    uint32_t _index;

	private : 

	    friend Generator;

	    TupleAccess ();

	    TupleAccess (const lexing::Word & loc, const Generator & type, const Generator & tuple, uint32_t index);

	public:

	    /**
	     * \brief Generate a tuple access
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & tuple, uint32_t index);
	    
	    Generator clone () const override;

	    bool equals (const Generator & other) const override;

	    const Generator & getTuple () const;

	    uint32_t getIndex () const;

	    std::string prettyString () const override;	    
	    
	};
	
    }
    
}
