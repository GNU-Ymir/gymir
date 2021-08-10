#pragma once

#include <ymir/semantic/generator/value/Aliaser.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Binary 
	 * Transform a value to a reference to this value
	 * The value must be a lvalue
	 */
	class DeepCopy : public Aliaser {
	private :

	    friend Generator;
	    
	    DeepCopy ();

	    DeepCopy (const lexing::Word & loc, const Generator & type, const Generator & who);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & who);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    std::string prettyString () const override;
	};
	
    }

}
