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
	     * \brief Create the loop that iterate over an array
	     * \param expression the syntaxic for loop
	     * \param array the value to iterate
	     * \param index the index var (could be empty)
	     * \param val the declaration of the iterator (by values)
	     * \verbatim
	     for i, ref mut j in array { }
	     
	     // => iterateArray (<for...>, <array>, <i>, <ref mut j>)

	     for it in array { }
	     // => iterateArray (<for...>, <array>, empty, <it>)
	     \endverbatim
	     */
	    generator::Generator iterateArray (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & index, const syntax::Expression & val);

	    /**
	     * Validate the var declaration of the iterator in case of by value array iteration
	     */
	    generator::Generator validateArrayByValueIterator (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & val, const generator::Generator & value, int level);


	    /**
	     * \brief Create a vardecl, and var ref for array and slice indexing
	     * \param for the for loop
	     * \param name the name of the index var
	     * \param {vardecl, varref}
	     */
	    std::vector <generator::Generator> createIndexVar (const syntax::For & expression, const std::string & name);

	    /**
	     * \brief Create a vardecl, and var ref for array and slice indexing
	     * \param for the for loop
	     * \param name the name of the index var
	     * \param {vardecl, varref}
	     */
	    std::vector <generator::Generator> createIndexVar (const syntax::For & expression, const syntax::VarDecl & decl);	    
	    
	    
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
