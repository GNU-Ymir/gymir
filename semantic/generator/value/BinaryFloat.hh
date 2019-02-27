#pragma once

#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Binary 
	 * Binary operations on floats
	 */
	class BinaryFloat : public Binary {
	private :

	    friend Generator;
	    
	    BinaryFloat ();

	    BinaryFloat (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right);

	public :

	    /**
	     * \brief Generate a new Binary on float
	     * \warning left and right operand must generate float typed values
	     */
	    static Generator init (const lexing::Word & loc, Operator op, const Generator & type, const Generator & left, const Generator & right);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    

	};
	
    }

}
