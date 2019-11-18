#pragma once

#include <ymir/syntax/expression/Unary.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/Unary.hh>

namespace semantic {

    namespace validator {

	/**
	 * \struct UnaryVisitor
	 * This visitor is used to validate any unary expression
	 */
	class UnaryVisitor {

	    Visitor & _context;

	private : 
	    
	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    UnaryVisitor (Visitor & context);

	public :

	    /**
	     * \brief Create a new visitor
	     * \param context the context of the unary, (used to validate the operands)
	     */
	    static UnaryVisitor init (Visitor & context);

	    /**
	     * \brief Validate a unary expression and return a generator
	     * \brief A unary operation validation is done in two phases
	     * \brief - First, we try to perform the operation simply
	     * \brief - Second, we look for an override of the operators (iif operand is a class)
	     */
	    generator::Generator validate (const syntax::Unary & expression);
	    
	    /**
	     * \brief Validate the operation on a bool
	     * \param un the operation
	     * \param operand an operand (assumed to be an bool value)
	     */	    
	    generator::Generator validateBool (const syntax::Unary & un, const generator::Generator & operand);
	    
	    /**
	     * \brief Validate the operation on a char
	     * \param un the operation
	     * \param operand an operand (assumed to be an char value)
	     */	    
	    generator::Generator validateChar (const syntax::Unary & un, const generator::Generator & operand);
	    
	    
	    /**
	     * \brief Validate the operation on a float
	     * \param un the operation
	     * \param operand an operand (assumed to be an float value)
	     */	    
	    generator::Generator validateFloat (const syntax::Unary & un, const generator::Generator & operand);
	    
	    /**
	     * \brief Validate the operation on a integer 
	     * \param un the operation
	     * \param operand an operand (assumed to be an integer value)
	     */
	    generator::Generator validateInt (const syntax::Unary & un, const generator::Generator & operand);

	    /**
	     * \brief Validate the operation on a pointer
	     * \param un the operation
	     * \param operand an operand (assumed to be pointer value)
	     */
	    generator::Generator validatePointer (const syntax::Unary & un, const generator::Generator & operand);

	    /**
	     * \brief Validate a function pointer from frame proto
	     * \param un the operation
	     * \param proto the frame prototype
	     */
	    generator::Generator validateFunctionPointer (const syntax::Unary & un, const generator::Generator & proto);
	    
	private :

	    /**
	     * \brief Transform the token into the right operator
	     */
	    generator::Unary::Operator toOperator (const lexing::Word & loc);	    

	    /**
	     * Throw an undefined op error
	     */
	    static void error (const syntax::Unary & un, const generator::Generator & operand);
	    
	};	

    }
    
}
