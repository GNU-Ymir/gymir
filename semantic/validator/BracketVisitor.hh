#pragma once

#include <ymir/syntax/expression/MultOperator.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/ArrayAccess.hh>

namespace semantic {

    namespace validator {

	/**
	 * \struct BracketVisitor
	 * This visitor is responsible of the validation of the bracket operator
	 * In Ymir, there are only two types that can use brackets operator : 
	 * - Array
	 * - Slice
	 * For the following types the operator can be overriden using the function name opIndex
	 * - Class
	 */
	class BracketVisitor {

	    Visitor & _context;

	private :
	    
	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    BracketVisitor (Visitor & context);

	public : 

	    /**
	     * \brief Create a new Visitor
	     * \param context the context of the bracket visitor (used to validate the operands)
	     */
	    static BracketVisitor init (Visitor & context);

	    /**
	     * \brief Validate a bracket expression and return a generator
	     * \brief This validation is done as follow : 
	     * \brief If left operand type is array : 
	     * \brief - Apply slice operation and return the generator 
	     * \brief Else If left operand is Slice : 
	     * \brief - Apply slice operation and return the generator 
	     * \brief Else If left operation is Class : 
	     * \brief - Search for operator overloading
	     * \brief Else throw an error
	     */
	    generator::Generator validate (const syntax::MultOperator & expression);

	    /**
	     * \brief Validate the operation done on an array
	     * \param expression the expression produce by the syntax analyses
	     * \param left the left operand returned by semantic validation
	     * \param rights the operands at the right of the expression
	     */
	    generator::Generator validateArray (const syntax::MultOperator & expression, const generator::Generator & left, const std::vector <generator::Generator> & rights);

	    /**
	     * \brief Validate the operation done on a string literal
	     * \param expression the expression produce by the syntax analyses
	     * \param left the left operand returned by semantic validation
	     * \param rights the operands at the right of the expression
	     */
	    generator::Generator validateStringLiteral (const syntax::MultOperator & expression, const generator::Generator & left, const std::vector <generator::Generator> & rights);
	    
	    /**
	     * \brief Validate the operation done on a slice
	     * \param expression the expression produce by the syntax analyses
	     * \param left the left operand returned by semantic validation
	     * \param rights the operands at the right of the expression
	     */
	    generator::Generator validateSlice (const syntax::MultOperator & expression, const generator::Generator & left, const std::vector <generator::Generator> & rights);

	    /**
	     * \brief Validate the operation done on a class
	     */
	    generator::Generator validateClass (const syntax::MultOperator & expression, const generator::Generator & left, const std::vector <generator::Generator> & rights);


	    /**
	     * Create the generator for the expression `throw OutOfArray::new ();` 
	     */
	    generator::Generator createThrowOutOfArray (const lexing::Word & location);

	    
	    /**
	     * Throw an undefined op error
	     */
	    static void error (const syntax::MultOperator & expression, const generator::Generator & left, const std::vector <generator::Generator> & rights);
	    
	};
	
    }
    
}
