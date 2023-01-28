#pragma once

#include <ymir/syntax/expression/MultOperator.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/semantic/generator/value/ArrayAccess.hh>

namespace semantic {

    namespace validator {

	/**
	 * \struct CallVisitor
	 * This visitor is responsible of the validation of the call operator
	 */
	class CallVisitor {

	    enum Scores {
		SCORE_TYPE = 3,
		SCORE_MUT = 1
	    };
	    
	    Visitor & _context;

	private :
	    
	    /**
	     *  As for any not proxied class, it does nothing special
	     *  We just wan't all class to be initialized the same way
	     */
	    CallVisitor (Visitor & context);

	public : 

	    /**
	     *  Create a new Visitor
	     * \param context the context of the call visitor (used to validate the operands)
	     */
	    static CallVisitor init (Visitor & context);

	    /**
	     *  Validate a call expression and return a generator
	     *  validate left, and params value, and then call the second validate function
	     */
	    generator::Generator validate (const syntax::MultOperator & expression);

	    /**
	     *  Validate a call expression and return a generator
	     *  This validation is done as follow : 
	     *  If left operand type is FrameProto : 
	     *  - Apply call of left operand
	     *  Else If left operand is Function Pointer : 
	     *  - Apply call of left operand
	     *  Else if left operand is MultSym
	     *  - Find the one with the best score
	     *  Else If left operation is Class : 
	     *  - Search for operator overloading
	     *  Else throw an error
	     */	    
	    generator::Generator validate (const lexing::Word & location, const generator::Generator & left, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors);


	    /**
	     *  Validate a mult sym 
	     *  A mult sym regroups a list of frameproto
	     *  It will use the more specialized one
	     */
	    generator::Generator validateMultSym (const lexing::Word & expression, const generator::MultSym & sym, const std::vector<generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors);	    

	    /**
	     * Validate the multsym generators
	     * @returns: 
	     *    - score: the highest score in the multsym
	     *    - nonTemplScores: the score of the non template element inside the multsym (score => validate (it))  
	     *    - templScores: the score of the template elements inside the multsym (score => validate (it))
	     *    - fromTempl: the highest score come from a template specialization
	     *    - final_gen: the generator with the highest score
	     *    - used_gen: the generator used for the validation
	     *    - templSym: the symbol created by template specialization
	     *    - proto_gen: the proto generator from template validation (if template specialization has the highest score)
	     *    - errors: the list of errors (valid only if final_gen is empty)
	     */
	    void computeScoreOfMultSym (const lexing::Word & location, const generator::MultSym & sym,  const std::vector <generator::Generator> & rights_, int & score, std::map <int, std::vector <generator::Generator>> & nonTemplScores, std::map <int, std::vector <Symbol>> & templScores, bool & fromTempl, generator::Generator & final_gen, generator::Generator & used_gen, Symbol & templSym, generator::Generator & proto_gen, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     * Validate the proto with the best score, and verify there is only one possibility	     
	     */
	    void validateScoreOfMultSym (const lexing::Word & location, const generator::MultSym & sym,  const std::vector <generator::Generator> & rights_, int & score, std::map <int, std::vector <generator::Generator>> & nonTemplScores, std::map <int, std::vector <Symbol>> & templScores, bool & fromTempl, generator::Generator & final_gen, generator::Generator & used_gen, Symbol & templSym, generator::Generator & proto_gen, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     * Compute the error list of a template validation (other calls, or verbose, etc.)
	     */
	    std::list <Ymir::Error::ErrorMsg> computeLastErrorList (const lexing::Word & location, Ymir::Error::ErrorList & errors, const generator::Generator & proto_gen, const generator::Generator & used_gen);
	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================      PROTOS     ==============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    /**
	     *  Validate a call expression on a frame proto
	     * \param expression the call expression 
	     * \param proto the prototype to use
	     * \param params the parameter that will be passed to the function 
	     * \param score the final score of the call 
	     *  It does not throw an exception on failure, 
	     *  It will return a empty generator 
	     */
	    generator::Generator validateFrameProto (const lexing::Word & location, const generator::Generator & proto, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     *  Validate a call expression on a constructor proto
	     * \param expression the call expression 
	     * \param proto the prototype to use
	     * \param params the parameter that will be passed to the function 
	     * \param score the final score of the call 
	     *  It does not throw an exception on failure, 
	     *  It will return a empty generator 
	     */	    
	    generator::Generator validateConstructorProto (const lexing::Word & location, const generator::Generator & proto, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors);

	    
	    /**
	     *  Validate a call expression on a lambda proto
	     * \param expression the call expression 
	     * \param proto the prototype to use
	     * \param params the parameter that will be passed to the function 
	     * \param score the final score of the call 
	     *  It does not throw an exception on failure, 
	     */
	    generator::Generator validateLambdaProto (const lexing::Word & location, const generator::Generator & proto, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors);

	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * ==================================     STRUCT     ==============================
	     * ================================================================================
	     * ================================================================================
	     */

	    
	    /**
	     *  Validate a call expression on a structure
	     * \param expression the call expression 
	     * \param str the structure to construct
	     * \param rights the parameter that will be passed to the constructor 
	     * \param score the final score of the call 
	     *  It does not throw an exception on failure, 
	     *  All the errors will be store into errors
	     */
	    generator::Generator validateStructCst (const lexing::Word & location, const generator::Struct & str, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     * Check if there are parameters with named generator that have no sense
	     * \param str the structure that is constructed
	     * \param params the list of parameters used to construct the struct
	     * \param errors the list of errors to fill
	     */
	    void checkStructNamedGen (const generator::Struct & str, const std::vector <generator::Generator> & params, std::list <Ymir::Error::ErrorMsg> & errors) const; 
	    
	    /**
	     *  Validate a call expression on a union structure
	     * \param expression the call expression 
	     * \param str the structure to construct
	     * \param rights the parameter that will be passed to the constructor 
	     * \param score the final score of the call 
	     *  It does not throw an exception on failure, 
	     *  All the errors will be store into errors
	     */
	    generator::Generator validateUnionCst (const lexing::Word & location, const generator::Struct & str, const std::vector <generator::Generator> & rights, int & score, std::list <Ymir::Error::ErrorMsg> & errors);
	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * ================================     POINTERS     ==============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     *  Validate a Function pointer call
	     * \param expression the call expression
	     * \param gen the generator of the function pointer value (assumed to be typed as FuncPtr)
	     * \param params the parameters of the call
	     *  It does not throw an exception on failure, 
	     *  It will return a empty generator 
	     *  All the errors will be store into errors
	     */
	    generator::Generator validateFunctionPointer (const lexing::Word & expression, const generator::Generator & gen, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors);


	    /**
	     *  Validate a Function pointer call
	     * \param expression the call expression
	     * \param gen the generator of the delegate value (assumed to be typed as Delegate)
	     * \param params the parameters of the call
	     *  It does not throw an exception on failure, 
	     *  It will return a empty generator
	     *  All the errors will be store into errors
	     * \warning TODO, merge this function and validateFunctionPointer, these two functions do exaclty the same treatment and have only one different code line 
	     */
	    generator::Generator validateDelegate (const lexing::Word & expression, const generator::Generator & gen, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================    TEMPLATES    ==============================
	     * ================================================================================
	     * ================================================================================
	     */

	    
	    /**
	     * Validate a template, and dispatch it between template class cst, and template ref
	     */
	    generator::Generator validateTemplate (const lexing::Word & expression, const generator::Generator & ref, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors, Symbol & sym, generator::Generator & proto_gen);	    
	    
	    /**
	     *  Validate a template ref 
	     *  a template ref can be called if refer to a function 
	     *  It will start by solving the template params with the types of the parameters
	     */
	    generator::Generator validateTemplateRef (const lexing::Word & expression, const generator::Generator & ref, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors, Symbol & sym, generator::Generator & proto_gen);

	    /**
	     *  Validate a template Class Cst
	     */
	    generator::Generator validateTemplateClassCst (const lexing::Word & expression, const generator::Generator & ref, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors, Symbol & sym, generator::Generator & proto_gen);

	    /**
	     *  Validate a template class cst prototype
	     */
	    generator::Generator validateTemplateClassCst (const lexing::Word & expression, const generator::Generator & ref, const syntax::Function::Prototype & prototype, const std::vector <generator::Generator> & params, int & score, std::list <Ymir::Error::ErrorMsg> & errors, Symbol & sym);

	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================    ARGUMENTS    ==============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    /**
	     * Return the list of the types of the parameters
	     * If verifyType, then verify the compatibility
	     * If verifyMemory, then verify the memory borrowing
	     * If isLambda, then some parameter can have no type, then the type of the argument is used
	     * If isTypeList, then the prototype is composed of only types, otherwise they are values
	     */
	    std::vector <generator::Generator> validateTypeParameterList (const generator::Generator & frame, const std::vector <generator::Generator> & proto, const std::vector <generator::Generator> & params, int & score, bool verifyType, bool verifyMemory, bool isLambda, bool isTypeList, std::list<Ymir::Error::ErrorMsg> & errors);
		    
	    /**
	     * Create the list of parameters (resolving the location of each parameter, taking into account the named params)
	     */
	    std::vector <generator::Generator> validateParameterList (const std::vector <generator::Generator> & proto, std::list <generator::Generator> & rights, std::list<Ymir::Error::ErrorMsg> & errors);
	    
	    /**
	     *  Find the right parameter assoc to the var
	     */
	    generator::Generator findParameter (std::list <generator::Generator> & params, const generator::ProtoVar & var);

	    /**
	     * Find a parameter in param list, for a proto var whose nbConsume is equal to 1
	     */
	    generator::Generator findSingleParameter (std::list <generator::Generator> & params, const generator::ProtoVar & var);

	    /**
	     *  Find the right parameter assoc to the var
	     */
	    generator::Generator findParameterStruct (std::vector<generator::Generator> & params, const generator::VarDecl & var);	    
	    
	    	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================     DOTCALL     ==============================
	     * ================================================================================
	     * ================================================================================
	     */


	    /**
	     *  Try to validate a dotCall expression
	     * \param left the dot expression (or something else that will fail in any case, but this is handled)
	     * \param params the parameter to fill with the left expression (a.b ()-> return b; and params.push (a))
	     * \param errors the errors to throw in case of failure
	     */
	    generator::Generator validateDotCall (const syntax::Expression & left, std::vector<generator::Generator> & params, const std::list <Ymir::Error::ErrorMsg> & errors);



	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================      ERRORS     ==============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    /**
	     * Throw an undefined op error
	     */
	    static void error (const lexing::Word & expression, const generator::Generator & left, const std::vector <generator::Generator> & rights, const std::list <Ymir::Error::ErrorMsg> & errors);

	    static void error (const lexing::Word & expression, const lexing::Word & end, const generator::Generator & left, const std::vector <generator::Generator> & rights, const std::list <Ymir::Error::ErrorMsg> & errors);

	    static std::string prettyName (const generator::Generator & gen);

	    static std::string prettyTypeName (const generator::Generator & gen);

	    static lexing::Word realLocation (const generator::Generator & gen);

	    static bool parameterExists (const std::vector <generator::Generator> & proto, const std::string & name);
	    
	};
	
    }
    
}
