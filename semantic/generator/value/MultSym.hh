#pragma once

#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Binary 
	 * Binary operations on ints
	 */
	class MultSym : public Value {

	    std::vector <Generator> _gens;

	private :

	    friend Generator;
	    
	    MultSym ();

	    MultSym (const lexing::Word & loc, const std::vector <Generator> & gens, bool crashIfZero);

	public :

	    /**
	     * \brief Generate a new multSym
	     */
	    static Generator init (const lexing::Word & loc, const std::vector <Generator> & gens, bool crashIfZero = true);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the list of generators of the mult reference
	     */
	    const std::vector <Generator> & getGenerators () const;

	    std::string prettyString () const override;
	    
	};
	
    }

}
