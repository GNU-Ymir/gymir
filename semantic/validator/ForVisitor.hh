#pragma once

#include <ymir/syntax/expression/For.hh>
#include <ymir/semantic/validator/Visitor.hh>


namespace semantic {

    namespace validator {

	/**
	 * \struct ForVisitor
	 * This is visitor is responsible of the validation of a for loop expression
	 * In Ymir, there is mutliple types, that can be iterated : 
	 * - Array
	 * - Slice
	 * - Range
	 * - ...
	 * For the following types, the operator can be overriden using an iterator type 
	 * - Class
	 */
	class ForVisitor {

	    Visitor & _context;

	private :

	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    ForVisitor (Visitor & context);

	public :

	    static ForVisitor init (Visitor & context);

	    /**
	     * \brief Validate a for loop
	     * \return a generator
	     */
	    generator::Generator validate (const syntax::For & expression);

	    /**
	     * \brief Validate a for loop on an array
	     */
	    generator::Generator validateArray (const syntax::For & expression, const generator::Generator & value);

	    /**
	       
	     */
	    generator::Generator iterateArrayByValue (const syntax::For & expression, const generator::Generator & value, const syntax::Expression & val);

	    /**
	     * Validate the var declaration of the iterator in case of by value array iteration
	     */
	    generator::Generator validateArrayByValueIterator (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & val, const generator::Generator & value, int level);
	    
	    
	    generator::Generator iterateArrayByIndexAndValue (const syntax::For & expression, const generator::Generator & value, const syntax::Expression & index, const syntax::Expression & val);

	    
	    /**
	     * \brief Validate a for loop on a slice
	     */
	    generator::Generator validateSlice (const syntax::For & expression, const generator::Generator & value);


	    /**
	     * Throw an undefined op error
	     */
	    static void error (const syntax::For & expression, const generator::Generator & value);
	};

	
    }
    
}
