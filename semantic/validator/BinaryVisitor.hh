#pragma once

#include <ymir/syntax/expression/Binary.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace validator {
       	
	/**
	 * \struct BinaryVisitor
	 * This visitor is used to validate any binary expression 
	 * A binary expression is pretty complex and depends on the type 
	 * That's why a special visitor is used
	 */
	class BinaryVisitor {

	    Visitor & _context;
	    
	private :

	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    BinaryVisitor (Visitor & context);

	public :

	    /**
	     * \brief Create a new visitor
	     * \param context the context of the binary, (used to validate the operands)
	     */
	    static BinaryVisitor init (Visitor & context);

	    /**
	     * \brief Validate a binary expression and return a generator
	     * \brief A binary operation validation is done in multiple phases
	     * \brief - First, we try to perform the operation from the left operand
	     * \brief - Second, if the first validation failed, we perform the operation from the right operand
	     * \brief - Third, if these two phases failed, we look for an override of the operators	     
	     */
	    generator::Generator validate (const syntax::Binary & expression) ;	    	    

	    /**
	     * \brief Validate a mathematic operation from two operands 
	     * \brief mathematics operations are in opposition to logical operations
	     */
	    generator::Generator validateMathOperation (generator::Binary::Operator op, const syntax::Binary & expression);

	    /**
	     * \brief Validate a mathematic operation from two operands 
	     * \brief mathematics operations are in opposition to logical operations
	     */
	    generator::Generator validateMathOperation (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a binary math operation with an integer on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we assume to be an Integer value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateMathIntLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a binary math operation with an char on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we assume to be an Integer value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateMathCharLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);
	    
	    /**
	     * \brief Validate a binary math operation with a float on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we assume to be a Float value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateMathFloatLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a binary math operation with a ptr on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we assume to be a Pointer value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateMathPtrLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);
	    
	    /**
	     * \brief Validate a binary math operation with an array on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we assume to be an Array value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateMathArray (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);


	    /**
	     * \brief Validate a binary math operation with an slice on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we assume to be a Slice value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateMathSlice (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a binary math operator with a class as the left operand
	     * \param op the operator of the expression
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we assume to be a Class value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateMathClass (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);


	    /**
	     * \brief Validate a binary math operator with a class as the right operand
	     * \param op the operator of the expression
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we don't know the type yet
	     * \param right the right operand that we assume to be a Class value
	     */
	    generator::Generator validateMathClassRight (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);


	    /**
	     * \brief Validate a binary math operator with a struct as the left operand
	     * \param op the operator of the expression
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we assume to be a Struct value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateMathStruct (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);


	    /**
	     * \brief Validate a binary math operator with a struct as the right operand
	     * \param op the operator of the expression
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand that we don't know the type yet
	     * \param right the right operand that we assume to be a Struct value
	     */
	    generator::Generator validateMathStructRight (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    

	    /**
	     * \brief Validate a logical operation from two operands
	     * \brief Logical operations are operations that typically create boolean
	     */
	    generator::Generator validateLogicalOperation (generator::Binary::Operator op, const syntax::Binary & expression);

	    /**
	     * \brief Validate a binary logical operation with an integer on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand assumed to be an Integer value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateLogicalIntLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);


	    /**
	     * \brief Validate a binary logical operation with an char on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand assumed to be an Char value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateLogicalCharLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);
	    
	    /**
	     * \brief Validate a binary logical operation with a float on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand assumed to be a Float value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateLogicalFloatLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);
	    
	    /**
	     * \brief Validate a binary logical operation with a bool on the left operand
	     * \param op the operator of the expression 
	     * \param expression the syntaxic node that will produce the generator
	     * \param left the left operand assumed to be an Bool value
	     * \param right the right operand that we don't know the type yet
	     */
	    generator::Generator validateLogicalBoolLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);


	    /**
	     * \brief Validate a binary logical operator with an array on left operator
	     */
	    generator::Generator validateLogicalArrayLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a binary logical operator with a slice on left operand
	     */
	    generator::Generator validateLogicalSliceLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a binary logical operator with a slice on left operand
	     */
	    generator::Generator validateEqualSliceLeft (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a binary logical operator with a class on left operand
	     */
	    generator::Generator validateLogicalClass (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a equality operator with a class on left operand
	     */
	    generator::Generator validateEqualClass (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);
	    
	    /**
	     * \brief Validate a binary logical operator with a class on right operand
	     */
	    generator::Generator validateLogicalClassRight (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a equality operator with a class on right operand
	     */
	    generator::Generator validateEqualClassRight (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);
	    
	    /**
	     * \brief Validate a binary logical operator with a class on left operand
	     */
	    generator::Generator validateLogicalStruct (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a equality operator with a class on left operand
	     */
	    generator::Generator validateEqualStruct (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);
	    
	    /**
	     * \brief Validate a binary logical operator with a class on right operand
	     */
	    generator::Generator validateLogicalStructRight (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    /**
	     * \brief Validate a equality operator with a class on right operand
	     */
	    generator::Generator validateEqualStructRight (generator::Binary::Operator op, const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);

	    
	    /**
	     * \brief Validate an affectation, the pre operation is a mathetical one (or concat, but its considered math)
	     * \param op the additional operation to perfom before affectation (can be LAST_OP, meaning no pre operation)
	     * \param expression the binary expression 
	     */
	    generator::Generator validateAffectation (generator::Binary::Operator op, const syntax::Binary & expression);

	    /**
	     * \brief Validate an index assign, a [i] = b
	     */
	    generator::Generator validateIndexAssign (generator::Binary::Operator op, const syntax::Binary & expression);

	    /**
	     * \Brief Validate an unary op assign, *a = b
	     */
	    generator::Generator validateUnaryAssign (generator::Binary::Operator op, const syntax::Binary & expression);
	    
	    /**
	     * \brief Validate an affectation on complex type (arrays, tuple, struct ...)
	     * \brief implicit Copy and reference are forbidden
	     * \brief This ensure that right is a Referencer or a Copier
	     */
	    generator::Generator validateAffectationCopy (const syntax::Binary & expression, const generator::Generator & left, const generator::Generator & right);


	    /**
	     * \brief Validate a range operation 
	     * \param op the operator of the expression
	     * \param expression the syntaxic node 
	     */
	    generator::Generator validateRangeOperation (generator::Binary::Operator op, const syntax::Binary & expression);


	    /**
	     * \brief Validate a pointer operation 
	     * \param op the operator of the expression
	     * \param expression the syntaxic node 
	     */
	    generator::Generator validatePointerOperation (generator::Binary::Operator op, const syntax::Binary & expression);


	    /**
	     * \brief Validate a contain operation
	     * \param op the operator of the expression
	     * \param expression 
	     */
	    generator::Generator validateContain (generator::Binary::Operator op, const syntax::Binary & expression);
	    
	    	    
	    /**
	     * \brief Transform a token into a binary operator
	     * \param word the token containing the op
	     * \param isAff returned value, set to true if this operator is a affectation
	     */
	    static generator::Binary::Operator toOperator (const lexing::Word & word, bool & isAff);

	private :
	    
	    /**
	     * \return is this operator a mathematic operator
	     */
	    bool isMath (generator::Binary::Operator op);

	    /**
	     * \return is this operator a logic operator
	     */
	    bool isLogical (generator::Binary::Operator op);	    

	    /**
	     * \return is this operator a range operator
	     */
	    bool isRange (generator::Binary::Operator op);
	    
	    /**
	     * \return is this operator a pointer operator
	     */
	    bool isPointer (generator::Binary::Operator op);

	    /**
	     * \return is this operator a contain operator
	     */
	    bool isContain (generator::Binary::Operator op);

	    std::string toString (generator::Binary::Operator op);
	    
	    /**
	     * Use to inverse logical operation (for opTest on class only)
	     */
	    std::string inverseOperator (generator::Binary::Operator op);
	    
	};	
	
    }
    
}
