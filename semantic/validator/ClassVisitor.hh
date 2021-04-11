#pragma once

#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/type/_.hh>
#include <ymir/semantic/validator/FunctionVisitor.hh>

namespace semantic {

    namespace validator {

	class ClassVisitor {

	    Visitor & _context;

	    FunctionVisitor _funcVisitor;
	public :

	    static std::list <semantic::Symbol>  __validation__;
	    static std::list <semantic::Symbol>  __post_validation__;
	    
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
	     * Actually validates the class content (vtable, fields, ...)
	     * This function is called if the class is not already validated
	     */
	    void validateClassContent (const semantic::Symbol & cls, bool inModule = false);

	    /**
	     * Revalidate the vtable of class when all the vtable are correctly set
	     */
	    void postValidate (const semantic::Symbol & sym);

	    /**
	     * Validate all the class needing a post validation
	     */
	    void postValidateAll ();
	    
	    /**
	     * Throw the errors and set an error symbol to Sym to avoid multiple validation of an invalid class type
	     */
	    void throwErrors (semantic::Symbol & sym, const std::list <Ymir::Error::ErrorMsg> & errors) const;
	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================     FIELDS     ===============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * Validate the fields of the class
	     * If !inModule, then the validation will be fast
	     */
	    void validateFields (const semantic::Symbol & cls, const generator::Generator & ancestor, bool inModule, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     * Validate the fields directly declared inside the class (not the ancestor fields)
	     * If !inModule, then the validation will be fast
	     */
	    std::vector <generator::Generator> validateLocalFields (const semantic::Symbol & cls, bool inModule, std::list <Ymir::Error::ErrorMsg> & errors);

	    /**
	     * Verify that there is no collisions with the fields of the ancestors
	     */
	    void verifyCollisions (const semantic::Symbol & cls, const std::vector <generator::Generator> & locals, const std::vector <generator::Generator> & ancetors);
	    
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
	     * Verify that there are no collision inside the vtable (two function with the same prototype)
	    */
	    void verifyCollisionVtable (const semantic::Symbol & cls, const generator::Generator & classType, const std::vector <generator::Generator> & vtable);
	    
	    /**
	     * Validate the inner class declaration of the class (for vtable only)
	     * @returns: the list of function prototypes of the vtable (without __dtor)
	     */
	    std::vector<generator::Generator> validateClassDeclarations (const semantic::Symbol & cls, const generator::Generator & classType, const generator::Generator & ancestor, std::vector <generator::Class::MethodProtection>  & protection, std::vector <Symbol> & addMethods);


	    /**
	     * Validate the inner declaration of an impl (for vtable only)
	     */
	    void validateVtableImpl (const semantic::Impl & impl, const generator::Generator & trait, const generator::Generator & classType, const generator::Generator & ancestor, std::vector <generator::Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <generator::Generator> & implVtable, std::vector<Symbol> & addMethods);

	    /**
	     * Validate the inner declaration of a trait (for vtable only)
	     * @returns: 
	     *    - addMethods: the list of methods to add to the vtable, and to validated
	     */
	    generator::Generator validateVtableTrait (const semantic::Impl & impl, const generator::Generator & classType, const generator::Generator & ancestor, std::vector <generator::Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <generator::Generator> & ancVtable, std::vector<Symbol> & addMethods);

	    /**
	     * Validate the methods of a trait (for vtable only)
	     * @returns: the index in the vtable of the method
	     */
	    int validateVtableTraitMethod (const semantic::Function & func, const generator::Generator & classType, const generator::Generator &, std::vector <generator::Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <generator::Generator> & ancVtable, const generator::Generator & trait);

	    /**
	     * Validate the methods (for vtable only)
	     * @returns: the index in the vtable of the method
	     */
	    int validateVtableMethod (const semantic::Function & func, const generator::Generator & classType, const generator::Generator &, std::vector <generator::Generator> & vtable, std::vector <generator::Class::MethodProtection> & protection, const std::vector <generator::Generator> & ancVtable, const generator::Generator & trait);	    


	    /**
	     * Validate the vtable symbol of the destructor
	     */
	    generator::Generator validateVtableDtor (const semantic::Symbol & cls, const generator::Generator & classGen, const generator::Generator & ancestor);

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
	     * Validate the inner symbols of a class, whose vtable and fields are already validated
	     */
	    void validateClassFull (const semantic::Symbol & cls);
	    
	    /**
	     * Validate the content symbol of the class (constructors, and methods)
	     */
	    void validateInnerClass (const semantic::Symbol & cls, std::list <Ymir::Error::ErrorMsg> & errors);

	    


	};
	
    }
    
}
