#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct OptionValue
	 * OptionValue is an array literal
	 */
	class OptionValue : public Value {

	    Generator _content;
	    bool _success;

	private :

	    friend Generator;
	    
	    OptionValue ();

	    OptionValue (const lexing::Word & loc, const Generator & type, const Generator & content, bool success);

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & content, bool success);
	    
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
	    const Generator & getContent () const;

	    /**
	     * \return true if this option value is a success or false if it is a failure
	     */
	    bool isSuccess () const;
	    
	    std::string prettyString () const override;
	};
	
    }

}
