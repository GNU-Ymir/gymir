#pragma once

#include <ymir/syntax/expression/Match.hh>
#include <ymir/semantic/validator/Visitor.hh>

namespace semantic {

    namespace validator {

	/**
	 * \struct MatchVisitor
	 * This visitor is responsible of the validation of the match operator
	 */
	class MatchVisitor {
	    
	    Visitor & _context;

	private :
	    
	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    MatchVisitor (Visitor & context);

	public : 

	    /**
	     * \brief Create a new Visitor
	     * \param context the context of the match visitor (used to validate the operands)
	     */
	    static MatchVisitor init (Visitor & context);

	    /**
	     * \brief Validate a match expression and return a generator
	     */
	    generator::Generator validate (const syntax::Match & expression);

	    /**
	     * \brief Validate a matcher
	     * \returns test, the validated test
	     * \warning this will throw errors if failure, that have to be caught elsewhere to follow the behavior of a pattern matcher
	     */
	    generator::Generator validateMatch (const generator::Generator & value, const syntax::Expression & matcher, bool & isMandatory);

	    /**
	     * \brief Validate a VarDecl
	     * \returns test, the validated test
	     */
	    generator::Generator validateMatchVarDecl (const generator::Generator & value, const syntax::VarDecl & decl, bool & isMandatory);

	    /**
	     * \brief Validate a binaryExpr
	     * \brief If it is a '|' or a '..' or '...', it must be validated by the pattern match otherwise it is validate normaly as any other expression
	     */
	    generator::Generator validateMatchBinary (const generator::Generator & value, const syntax::Binary & bin, bool & isMandatory);

	    /**
	     * \brief Valdiate a list expression
	     * \brief This is used to match over a tuple, or a static array
	     */
	    generator::Generator validateMatchList (const generator::Generator & value, const syntax::List & lst, bool & isMandatory);
	    
	    /**
	     * \brief Validate anything else, that is not a pattern matching
	     */
	    generator::Generator validateMatchAnything (const generator::Generator & value, const syntax::Expression & any, bool & isMandatory);
	    
	};
	    
    }
    
}
