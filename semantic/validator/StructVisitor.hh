#pragma once

#include <ymir/semantic/symbol/_.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/errors/_.hh>
#include <ymir/syntax/visitor/Keys.hh>

namespace semantic {

    namespace validator {
       	
	/**
	 * \struct StructVisitor
	 */
	class StructVisitor {

	    Visitor & _context;


	private:
	    
	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    StructVisitor (Visitor & context);

	public:

	    /**
	     * \brief Create a new visitor
	     * \param context the context of the operator (used to validate the operands)
	     */	    
	    static StructVisitor init (Visitor & context);

	    /**
	     * Validate a structure symbol
	     */
	    generator::Generator validate (const semantic::Symbol & sym);

	    /**
	     * Validate the content of the structure, this function is
	     * called when the structure is not already validated
	     */
	    void validateStructContent (const semantic::Symbol & sym);
	    
	    /**
	     * Verify that the struct is not self contained, and thus that its size is not infinite
	     */
	    void verifyRecursivity (const lexing::Word & loc, const generator::Generator & str, const semantic::Symbol & sym, bool fst) const;
	    
	};

    }

}
