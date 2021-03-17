#pragma once

#include <ymir/semantic/symbol/_.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/errors/_.hh>
#include <ymir/syntax/visitor/Keys.hh>

namespace semantic {

    namespace validator {
       	
	/**
	 * \struct FunctionVisitor
	 */
	class FunctionVisitor {

	    Visitor & _context;

	private :

	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    FunctionVisitor (Visitor & context);

	public :

	    /**
	     * \brief Create a new visitor
	     * \param context the context of the operator 
	     */
	    static FunctionVisitor init (Visitor & context);

	    /**
	     * Validate a function
	     */
	    void validate (const semantic::Function & func);


	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================      BODY      ===============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * Validate the body of a function
	     * And do some verifications: 
	     *   - finalReturn
	     *   - throwers
	     */
	    generator::Generator validateBody (const lexing::Word & loc, const std::string & funcName, const syntax::Expression & body, const std::vector <generator::Generator> & throwers, const generator::Generator & retType, bool & needFinalReturn, std::list <Ymir::Error::ErrorMsg> & errors);



	    /**
	     * Verify that the function has a final return, or that the value of the body correspond to the value of the function
	     */
	    bool verifyFinalReturn (const lexing::Word & loc, const generator::Generator & retType, const generator::Generator & body);

	    
	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * ================================    PROTO UTILS   ==============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * Verify that the prototype can be used for the prototype of the main function
	     */
	    void verifyMainPrototype (const lexing::Word & loc, const syntax::Function::Prototype & proto,  const std::vector <generator::Generator> & params, const generator::Generator & retType, std::list <Ymir::Error::ErrorMsg> & errors);
	    	    
	    /**
	     * Validate the type and value of a parameter decl
	     * (TODO, maybe its similar to simple var decl value, must check for factorization)
	     * @params: 
	     *   - no_value: if true, does not validate the value
	     * @returns: 
	     *    - type: the type of the parameter
	     *    - value: the value of the parameter
	     *    - isMutable: is the parameter mutable
	     */
	    void validateParamDecl (const lexing::Word & loc, const syntax::VarDecl & var, bool no_value, generator::Generator & type, generator::Generator & value, bool & isMutable);


	    /**
	     * Validate the return type of a prototype
	     */
	    generator::Generator validateReturnType (const lexing::Word & loc, const syntax::Expression & ret);


	    /**
	     * Verify parameter shadowing and insert them inside the current block
	     */
	    void insertParameters (const std::vector <generator::Generator> & params);


	    /**
	     * \brief Transform a frameProto into a FuncPtr type
	     */
	    generator::Generator validateFunctionType (const generator::Generator & proto);
	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * ================================      PROTO      ===============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    /**
	     * \brief Validate the prototype of a function in order to
	     * refer to it 
	     * A prototype stores the informations about
	     * the parameters, and return type, but does not contain
	     * the body of the function 
	     * \param func the function prototype to validate
	     */
	    generator::Generator validateFunctionProto (const semantic::Function & func);

	    /**
	     * \brief Validate the prototyp of a constructor in order to refer to it
	     * \param cs the constructor to validate
	     */
	    generator::Generator validateConstructorProto (const semantic::Symbol & cs);

	    /**
	     * \brief Validate the method prototype of a method in order to refer to it
	     * \param mt the method to validate
	     */
	    generator::Generator validateMethodProto (const semantic::Function & mt, const generator::Generator &classType, const generator::Generator & trait);
	    

	    /**
	     * \brief Validate the prototype of a function for the creation of a frameproto or a constructProto
	     * \param loc the location of the function
	     * \param proto the prototype of the function
	     * \return param params (returned by ref), the validated parameter of the function
	     * \return param retType (returned by ref), the validated type of the prototype (if any)
	     * \param no_value if it is true, the var_decl of the params cannot have values (they won't be validated)
	     * \warning we assume we enter a block, that will contains only the params
	     * \warning it is not entered inside this function, because for the case of a methodproto, we want to add the self var before hand
	     */
	    void validatePrototypeForProto (const lexing::Word & loc, const syntax::Function::Prototype & proto, bool no_value, std::vector <generator::Generator> & params, generator::Generator & retType, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**	      	       
	     * \brief Validate the prototype of a function for the creation of a frame (function or constructor)
	     * \param loc the location of the function
	     * \param proto the prototype of the function
	     * \return param params (returned by ref), the validated parameter of the function
	     * \return param retType (returned by ref), the validated type of the prototype (if any)
	     * \param no_value if it is true, the var_decl of the params cannot have values (they won't be validated)
	     */
	    void validatePrototypeForFrame (const lexing::Word & loc, const syntax::Function::Prototype & proto, std::vector <generator::Generator> & params, generator::Generator & retType, std::list <Ymir::Error::ErrorMsg> & errors);
	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * ==============================      THROWING      ==============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    /**
	     * Verify that the throwing of the function are correctly informed
	     */
	    void verifyThrowing (const lexing::Word & loc, const std::string & funcName, const std::vector <generator::Generator> & throwers, const std::vector <generator::Generator> & asked, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     * Compute the list of throwing that are unused (declared throws, but not thrown inside the body)
	     * , and the list of those that are not found (found in the body, but not declared)
	     * @params: 
	     *     - types: the list of throwers found inside the body
	     *     - rethrow: the list of throws declared inside the prototype
	     */
	    void computeThrows (const std::vector <generator::Generator> & types, const std::vector <generator::Generator> & rethrow, std::vector <generator::Generator> & unused, std::vector <generator::Generator> & notfound);

	    
	    /**
	     * Validate the type of the throwers of a function
	     */
	    std::vector <generator::Generator> validateThrowers (const std::vector <syntax::Expression> & throwers, std::list <Ymir::Error::ErrorMsg> & errors);



	    /**
	     * ================================================================================
	     * ================================================================================
	     * ==============================     CONSTRUCTOR    ==============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * Validate a constructor of a class (complete validation)
	     * Insert the validated frame inside the validated frame of the module (for generation)
	     */
	    void validateConstructor (const semantic::Symbol & sym, const generator::Generator & classType_, const generator::Generator & ancestor, const std::vector <generator::Generator> & ancestorFields);

	    /**
	     * Validate the pre construction elements (super or self call, and field initialization)
	     */
	    generator::Generator validatePreConstructor (const semantic::Constructor & cs, const generator::Generator & classType, const generator::Generator & ancestor, const std::vector<generator::Generator> & ancestorFields);

	    /**
	     * Verify that there is no loop of constructors
	     */
	    void verifyConstructionLoop (const lexing::Word & location, const generator::Generator & call);


	    /**
	     * ================================================================================
	     * ================================================================================
	     * ==============================        METHOD      ==============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    /**
	     * Validate a metho of a class (complete validation)
	     * Insert the validated frame inside the validated frame of the module (for generation)
	     */
	    void validateMethod (const semantic::Function & func, const generator::Generator & classType_, bool isWeak = false);

	    
	};
    }
}
