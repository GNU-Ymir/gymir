#pragma once

#include <ymir/syntax/expression/MultOperator.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/ArrayAccess.hh>

namespace semantic {

    namespace validator {

	/**
	 * \struct CastVisitor
	 * This visitor is responsible of the validation of the cast operator
	 */
	class CastVisitor {

	    Visitor & _context;

	private :
	    
	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    CastVisitor (Visitor & context);

	public : 

	    /**
	     * \brief Create a new Visitor
	     * \param context the context of the cast visitor (used to validate the operands)
	     */
	    static CastVisitor init (Visitor & context);

	    /**
	     * \brief Validate a cast expression and return a generator
	     */
	    generator::Generator validate (const syntax::Cast & expression);


	    void error (const syntax::Cast & expression, const generator::Generator & type, const generator::Generator & value);
	    
	};
	
    }
    
}
