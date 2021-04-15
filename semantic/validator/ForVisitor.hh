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
	     * Validate a for loop that is executed at compile time	     
	     */
	    generator::Generator validateCte (const syntax::For & expression);

	    /**
	     * Validate the var declaration of the iterator in case of by value array iteration
	     */
	    generator::Generator validateArrayByValueIterator (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & val, const generator::Generator & value, int level);


	    /**
	     * \brief Create a vardecl, and var ref for array and slice indexing
	     * \brief Declare as well a variable to store the value of the array to prevent multi creation of the var
	     * \param for the for loop
	     * \param value the array value
	     * \param name the name of the index var
	     * \param {arrayVarDecl, vardecl, arrayVarRef, varref}
	     */
	    std::vector <generator::Generator> createIndexVar (const syntax::For & expression, const generator::Generator & value, const std::string & name);

	    /**
	     * \brief Create a vardecl, and var ref for array and slice indexing
	     * \brief Declare as well a variable to store the value of the array to prevent multi creation of the var
	     * \param for the for loop
	     * \param value the array value
	     * \param decl the name of the index var (index var)
	     * \param {arrayVarDecl, vardecl, arrayVarRef, varref}
	     */
	    std::vector <generator::Generator> createIndexVar (const syntax::For & expression, const generator::Generator & value, const syntax::VarDecl & decl);	    
	    
	    
	    /**
	     * \brief Validate a for loop on a slice
	     */
	    generator::Generator validateSlice (const syntax::For & expression, const generator::Generator & value);


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
	    generator::Generator iterateSlice (const syntax::For & expression, const generator::Generator & array, const syntax::Expression & index, const syntax::Expression & val);
	    
	    generator::Generator validateRange (const syntax::For & expression, const generator::Generator & range, bool isCte = false);
	   	    
	    std::vector <generator::Generator> createIndexVarRange (const syntax::For & expression, const generator::Generator & range, const syntax::VarDecl & name);
	    
	    generator::Generator iterateRange (const syntax::For & expression, const generator::Generator & range, const syntax::Expression & index);

	    generator::Generator iterateRangeCte (const syntax::For & expression, const generator::Generator & range, const syntax::Expression & index);	    
	    
	    generator::Generator validateTuple (const syntax::For & expression, const generator::Generator & range);

	    std::vector <generator::Generator> createIndexVarTuple (const syntax::For & expression, const generator::Generator & range, const syntax::VarDecl & name, int level);

	    generator::Generator iterateTuple (const syntax::For & expression, const generator::Generator & range, const syntax::Expression & index);


	    /**
	     * Validate a for loop over a class
	     */
	    generator::Generator validateClass (const syntax::For & expression, const generator::Generator & value);

	    std::vector <generator::Generator> validateClassPreLoop (const syntax::For & expression, const generator::Generator & value, generator::Generator & iterRef, generator::Generator & endRef);
	    

	    generator::Generator validateIndexVarClass (const syntax::For&, const generator::Generator & iterator, const syntax::VarDecl & decl, int i);


	    generator::Generator validateLoopContent (const syntax::For & for_);
	    
	    /**
	     * Throw an undefined op error
	     */
	    static void error (const syntax::For & expression, const generator::Generator & value, bool isCte = false);
	};

	
    }
    
}
