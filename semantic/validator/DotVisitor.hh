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
	     * \param isFromCall if false, we ar allowed to make a dotCall to a templateCall ((10).to!string, for example)
	     */
	    generator::Generator validate (const syntax::Binary & expression, bool isFromCall);


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

	    /**
	     * \brief Validate the operator on a class type
	     * \param expression the bin expression
	     * \param left the value of type ClassRef
	     */
	    generator::Generator validateClass (const syntax::Binary & expression, const generator::Generator & left, std::vector <std::string> & errors);
	    
	    /**
	     * \brief Validate an implicit dotTemplateCall
	     * \brief a dotTemplateCall is as the following ((expression)'.' (var'!' params) [\'('], with 
	     * \brief Or the following AST : 
	     * \verbatim 
	     <Any not {Call}> 
	         <Dot> 
                       <Any>
		       <TemplateCall> 
	     \endverbatim
	     */
	    generator::Generator validateDotTemplateCall (const syntax::Binary & expression, const generator::Generator & left);

	private :

	    /**
	     * \brief Throw the error, UNDEFINED_FIELD_FOR
	     * \param expression the binary operator
	     * \param left the left element
	     * \param right the field we are trying to access
	     */
	    void error (const syntax::Binary & expression, const generator::Generator & left, const std::string & right, std::vector <std::string> & errors);
	    
	};

    }
}
