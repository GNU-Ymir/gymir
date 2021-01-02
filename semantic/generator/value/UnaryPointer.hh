#pragma once

#include <ymir/semantic/generator/value/Unary.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Unary 
	 * Unary operations on ints
	 */
	class UnaryPointer : public Unary {
	private :

	    friend Generator;
	    
	    UnaryPointer ();

	    UnaryPointer (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left);

	public :

	    /**
	     * \brief Generate a new Unary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & operand);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	};
	
    }

}
