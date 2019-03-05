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

	private :

	    void error (const syntax::Binary & expression, const generator::Generator & left, const std::string & right);
	    
	};

    }
}
