#pragma once

#include <ymir/syntax/expression/Binary.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace validator {
       	
	/**
	 * \struct SubVisitor
	 * The sub visitor class validate a '::' operator
	 */
	class SubVisitor {

	public : 
	    
	    static std::string __SIZEOF__;

	    static std::string __INIT__;

	    static std::string __TYPEID__;

	    static std::string __TYPEINFO__;
	    
	    Visitor & _context;

	private :

	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    SubVisitor (Visitor & context);

	public :

	    /**
	     * \brief Create a new visitor
	     * \param context the context of the operator (used to validate the operands)
	     */
	    static SubVisitor init (Visitor & context);

	    /**
	     * \brief Validate the operation and return a generator
	     * \brief A sub access done on a multsym, will return another multsym
	     */
	    generator::Generator validate (const syntax::Binary & expression);
	    	    
	    /**
	     * \brief Validate a multsym access
	     */
	    generator::Generator validateMultSym (const syntax::Binary &expression, const generator::MultSym & mult);

	    /**
	     * \brief Validate a module access
	     */
	    generator::Generator validateModuleAccess (const syntax::Binary &expression, const generator::ModuleAccess & acc);

	    /**
	     * \brief Validate an enum access
	     */
	    generator::Generator validateEnum (const syntax::Binary & expression, const generator::Enum & en);

	    /**
	     * \brief Validate a type access for standard values
	     */
	    generator::Generator validateType (const syntax::Binary & expression, const generator::Generator & type);
	    
	    generator::Generator validateArray (const syntax::Binary & expression, const generator::Generator & b);
	    generator::Generator validateBool (const syntax::Binary & expression, const generator::Generator & b);
	    generator::Generator validateChar (const syntax::Binary & expression, const generator::Generator & c);
	    generator::Generator validateFloat (const syntax::Binary & expression, const generator::Generator & f);
	    generator::Generator validateInteger (const syntax::Binary & expression, const generator::Generator & i);
	    generator::Generator validatePointer (const syntax::Binary & expression, const generator::Generator & p);
	    generator::Generator validateSlice (const syntax::Binary & expression, const generator::Generator & s);
	    generator::Generator validateTuple (const syntax::Binary & expression, const generator::Generator & t);
	    generator::Generator validateStruct (const syntax::Binary & expression, const generator::Generator & en);
	    generator::Generator validateClass (const syntax::Binary & expression, const generator::Generator & en, std::vector <std::string> & errors);
	    generator::Generator validateTemplate (const syntax::Binary & expression, const generator::Generator & en, std::vector <std::string> & errors);
	    generator::Generator validateClassValue (const syntax::Binary & expression, const generator::Generator & en);
	    generator::Generator validateStructValue (const syntax::Binary & expression, const generator::Generator & en);
	    
	private :	    

	    void error (const syntax::Binary & expression, const generator::Generator & left, const syntax::Expression & right);
	    void error (const syntax::Binary & expression, const generator::Generator & left, const syntax::Expression & right, std::vector <std::string> & errors);
	    
	};

    }
}
