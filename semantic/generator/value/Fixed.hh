#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * A block possess a list of expression 
	 * it has also a type, (as any value)
	 */
	class Fixed : public Value {

	public :
	    
	    union UI {
		uint64_t u;		
		int64_t i;
	    };
	    
	private :
	    
	    UI _value;
	    
	protected : 

	    friend Generator;

	    Fixed ();

	    Fixed (const lexing::Word & loc, const Generator & type, UI value);	    	    
	    
	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, UI value);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the value of the fixed const, can be signed or unsigned depending on the type
	     */
	    UI getUI () const;

	    std::string prettyString () const override;	    
	};	
	
	/**
	 * \brief Create a Fixed generator, of type usize
	 * \param value the content of the constant
	 */
	Generator ufixed (uint64_t value);

	/**
	 * \brief Create a fixed generator of type isize
	 * \param value the content of the constant
	 */
	Generator ifixed (int64_t value);

	/**
	 * \brief Create a Fixed generator, of type usize
	 * \param value the content of the constant
	 */
	Generator ufixed (const lexing::Word & location, uint64_t value);

	/**
	 * \brief Create a fixed generator of type isize
	 * \param value the content of the constant
	 */
	Generator ifixed (const lexing::Word & location, int64_t value);
    
    }

}

