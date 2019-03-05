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
	     */
	    generator::Generator validate (const syntax::Binary & expression);


	    /**
	     * \brief Validate the operator on a tuple value
	     * \param expression the bin operation
	     * \param left the value of type tuple
	     */
	    generator::Generator validateTuple (const syntax::Binary & expression, const generator::Generator & left);

	    generator::Generator validateStruct (const syntax::Binary & expression, const generator::Generator & left);

	private :

	    void error (const syntax::Binary & expression, const generator::Generator & left, const std::string & right);
	    
	};

    }
}
