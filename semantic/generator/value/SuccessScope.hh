#pragma once

#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Binary 
	 * Transform a value to a reference to this value
	 * The value must be a lvalue
	 */
	class SuccessScope : public Value {

	    Generator _who;

	    std::vector<Generator> _values;

	private :

	    friend Generator;
	    
	    SuccessScope ();

	    SuccessScope (const lexing::Word & loc, const Generator & type, const Generator & who, const std::vector<Generator> & values);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & who, const std::vector <Generator> & values);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /** 
	     * \return the left operand of the operation
	     */
	    const Generator & getWho () const;

	    /**
	     * \return the values to execute at the exit of the scope
	     */
	    const std::vector <Generator> & getValues () const;
	    
	    std::string prettyString () const override;
	    
	};
	
    }

}
