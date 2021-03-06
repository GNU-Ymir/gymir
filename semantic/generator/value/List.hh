#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct List
	 * List is an array literal
	 */
	class List : public Value {

	    std::vector <Generator> _content;

	private :

	    friend Generator;
	    
	    List ();

	    List (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /** 
	     * \return the content of the literal
	     */
	    const std::vector<Generator> & getParameters () const;

	    std::string prettyString () const override;
	};
	
    }

}
