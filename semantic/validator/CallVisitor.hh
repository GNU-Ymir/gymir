#pragma once

#include <ymir/syntax/expression/MultOperator.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/ArrayAccess.hh>

namespace semantic {

    namespace validator {

	/**
	 * \struct CallVisitor
	 * This visitor is responsible of the validation of the call operator
	 * In Ymir, there are only two types that can use calls operator : 
	 * - Array
	 * - Slice
	 * For the following types the operator can be overriden using the function name opIndex
	 * - Class
	 */
	class CallVisitor {

	    enum Scores {
		SCORE_TYPE = 3
	    };
	    
	    Visitor & _context;

	private :
	    
	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    CallVisitor (Visitor & context);

	public : 

	    /**
	     * \brief Create a new Visitor
	     * \param context the context of the call visitor (used to validate the operands)
	     */
	    static CallVisitor init (Visitor & context);

	    /**
	     * \brief Validate a call expression and return a generator
	     * \brief validate left, and params value, and then call the second validate function
	     */
	    generator::Generator validate (const syntax::MultOperator & expression);

	    /**
	     * \brief Validate a call expression and return a generator
	     * \brief This validation is done as follow : 
	     * \brief If left operand type is FrameProto : 
	     * \brief - Apply call of left operand
	     * \brief Else If left operand is Function Pointer : 
	     * \brief - Apply call of left operand
	     * \brief Else if left operand is MultSym
	     * \brief - Find the one with the best score
	     * \brief Else If left operation is Class : 
	     * \brief - Search for operator overloading
	     * \brief Else throw an error
	     */
	    generator::Generator validate (const syntax::MultOperator & expression, const generator::Generator & left, const std::vector <generator::Generator> & params, int & score, std::vector <std::string> & errors);
	    
	    /**
	     * \brief Validate a call expression on a frame proto
	     * \param expression the call expression 
	     * \param proto the prototype to use
	     * \param params the parameter that will be passed to the function 
	     * \param score the final score of the call 
	     * \brief It does not throw an exception on failure, 
	     * \brief It will return a empty generator and a score of -1
	     */
	    generator::Generator validateFrameProto (const syntax::MultOperator & expression, const generator::FrameProto & proto, const std::vector <generator::Generator> & params, int & score, std::vector<std::string> & errors);

	    /**
	     * \brief Find the right parameter assoc to the var
	     */
	    generator::Generator findParameter (std::vector<generator::Generator> & params, const generator::ProtoVar & var);
	    
	    /**
	     * \brief Validate a call expression on a frame proto
	     * \param expression the call expression 
	     * \param proto the prototype to use
	     * \param params the parameter that will be passed to the function 
	     * \param score the final score of the call 
	     * \brief It does not throw an exception on failure, 
	     * \brief It will return a empty generator and a score of -1
	     */
	    generator::Generator validateStructCst (const syntax::MultOperator & expression, const generator::Struct & str, const std::vector <generator::Generator> & params, int & score, std::vector<std::string> & errors);

	    /**
	     * \brief Find the right parameter assoc to the var
	     */
	    generator::Generator findParameterStruct (std::vector<generator::Generator> & params, const generator::VarDecl & var);	    
	    
	    /**
	     * \brief Validate a mult sym 
	     * \brief A mult sym regroups a list of frameproto
	     * \brief It will use the more specialized one
	     */
	    generator::Generator validateMultSym (const syntax::MultOperator & expression, const generator::MultSym & sym, const std::vector<generator::Generator> & params, int & score, std::vector<std::string> & errors);	    

	    /**
	     * \brief Validate a template ref 
	     * \brief a template ref can be called if refer to a function 
	     * \brief It will start by solving the template params with the types of the parameters
	     */
	    generator::Generator validateTemplateRef (const syntax::MultOperator & expression, const generator::TemplateRef & ref, const std::vector <generator::Generator> & params, int & score, std::vector <std::string> & errors, Symbol & sym, generator::Generator & proto_gen);


	    /**
	     * \brief Try to validate a dotCall expression
	     * \param left the dot expression (or something else that will fail in any case, but this is handled)
	     * \param params the parameter to fill with the left expression (a.b ()-> return b; and params.push (a))
	     * \param errors the errors to throw in case of failure
	     */
	    generator::Generator validateDotCall (const syntax::Expression & left, std::vector<generator::Generator> & params, const std::vector<std::string> & errors);
	    
	    /**
	     * Throw an undefined op error
	     */
	    static void error (const syntax::MultOperator & expression, const generator::Generator & left, const std::vector <generator::Generator> & rights, std::vector <std::string> & errors);

	    static void insertCandidate (int & nb, std::vector <std::string> & errors, const std::vector <std::string> & candErrors);
	    
	};
	
    }
    
}
