#pragma once

#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/type/_.hh>

namespace semantic {

    namespace validator {

	class ClassVisitor {

	    Visitor & _context;

	private :

	    ClassVisitor (Visitor & context);

	public :

	    /**
	     * Create a new visitor
	     * This class is not proxied
	     */
	    static ClassVisitor init (Visitor & content);

	    /**
	     * Validate the class symbol
	     */
	    generator::Generator validate (const semantic::Symbol & cls, bool inModule = false);

	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================     ASSERT     ===============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    void validateFields (const semantic::Symbol & cls, const generator::Generator & ancestor, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================     ASSERT     ===============================
	     * ================================================================================
	     * ================================================================================
	     */
	
	    
	    /**
	     * Validate the assertions inside the class
	     */
	    void validateCtes (const semantic::Symbol & cls);

	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================     VTABLE     ===============================
	     * ================================================================================
	     * ================================================================================
	     */
	
	    
	    /**
	     * Validate the vtable of the class
	     */
	    void validateVtable (const semantic::Symbol & cls, const generator::Generator & ancestor, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     * Validate the inner class declaration of the class (for vtable only)
	     * @returns: the list of function prototypes of the vtable (without __dtor)
	     */
	    std::vector<generator::Generator> validateClassDeclarations (const semantic::Symbol & cls, const generator::Generator & classType, const generator::Generator & ancestor, std::vector <generator::Class::MethodProtection>  & protection, std::vector <Symbol> & addMethods);

	    /**
	     * Validate the inner declaration of an implements (for vtable only)
	     * @returns: 
	     *    - addMethods: the list of methods to add to the vtable, and to validated
	     */
	    void validateVtableImplement (const semantic::Impl & impl, const generator::Generator & classType, const generator::Generator & ancestor, std::vector <generator::Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <generator::Generator> & ancVtable, std::vector<Symbol> & addMethods);

	    /**
	     * Validate the methods of a trait for impl (for vtable only)
	     * @returns: the index in the vtable of the method
	     */
	    int validateVtableMethodImplement (const semantic::Function & func, const generator::Generator & classType, const generator::Generator &, std::vector <generator::Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <generator::Generator> & ancVtable, const generator::Generator & trait);

	    /**
	     * Validate the methods (for vtable only)
	     * @returns: the index in the vtable of the method
	     */
	    int validateVtableMethod (const semantic::Function & func, const generator::Generator & classType, const generator::Generator &, std::vector <generator::Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <generator::Generator> & ancVtable, const generator::Generator & trait);	    


	    /**
	     * ================================================================================
	     * ================================================================================
	     * ================================     ANCESTOR     ==============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    /**
	     * Validate the expression of the ancestor class
	     */
	    generator::Generator validateAncestor (const semantic::Symbol & cls);


	    /**
	     * ================================================================================
	     * ================================================================================
	     * ================================      INNER      ===============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * Validate the content symbol of the class (constructors, and methods)
	     */
	    void validateInnerClass (const semantic::Symbol & cls, std::list <Ymir::Error::ErrorMsg> & errors);
	    
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
	     * Validate a metho of a class (complete validation)
	     * Insert the validated frame inside the validated frame of the module (for generation)
	     */
	    void validateMethod (const semantic::Function & func, const generator::Generator & classType_, bool isWeak = false);


	};
	
    }
    
}
