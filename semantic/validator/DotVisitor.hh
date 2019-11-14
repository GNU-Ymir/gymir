#pragma once

#include <ymir/syntax/expression/Binary.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace validator {
       	
	/**
	 * \struct DotVisitor
	 * The sub visitor class validate a '::' operator
	 */
	class DotVisitor {

	    Visitor & _context;

	private :

	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    DotVisitor (Visitor & context);

	public :

	    /**
	     * \brief Create a new visitor
	     * \param context the context of the operator (used to validate the operands)
	     */
	    static DotVisitor init (Visitor & context);

	    /**
	     * \brief Validate the operation and return a generator
	     */
	    generator::Generator validate (const syntax::Binary & expression);


	    /**
	     * \brief Validate the operator on a tuple value
	     * \param expression the bin operation
	     * \param left the value of type tuple
	     */
	    generator::Generator validateTuple (const syntax::Binary & expression, const generator::Generator & left);

	    /**
	     * \brief Validate the operator on a struct access
	     * \param expression the bin expression
	     * \param left the value of type struct
	     */
	    generator::Generator validateStruct (const syntax::Binary & expression, const generator::Generator & left);

	    /**
	     * \brief Validate the operator on a array access
	     * \param expression the bin expression
	     * \param left the value of type array
	     */
	    generator::Generator validateArray (const syntax::Binary & expression, const generator::Generator & left);

	    /**
	     * \brief Validate the operator on a slice access
	     * \param expression the bin expression
	     * \param left the value of type slice
	     */
	    generator::Generator validateSlice (const syntax::Binary & expression, const generator::Generator & left);


	    /**
	     * \brief Validate the operator on a range type 
	     * \param expression the bin expression
	     * \param left the value of type range
	     */
	    generator::Generator validateRange (const syntax::Binary & expression, const generator::Generator & left);	    
	    
	private :

	    /**
	     * \brief Throw the error, UNDEFINED_FIELD_FOR
	     * \param expression the binary operator
	     * \param left the left element
	     * \param right the field we are trying to access
	     */
	    void error (const syntax::Binary & expression, const generator::Generator & left, const std::string & right);
	    
	};

    }
}
