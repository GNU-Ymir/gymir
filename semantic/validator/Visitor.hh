#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/expression/_.hh>

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/symbol/_.hh>
#include <ymir/semantic/Generator.hh>
#include <ymir/semantic/generator/_.hh>
#include <ymir/utils/Match.hh>
#include <ymir/syntax/expression/VarDecl.hh>

#include <list>
#include <ymir/lexing/Word.hh>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>


namespace semantic {

    namespace validator {

	/**
	 * \struct Visitor
	 * This class is the final semantic validation before code production
	 * It will traverse all the declared symbol of a given module and validated each of them
	 */
	class Visitor {

	    /** List of referent for symbol access and insertion */
	    std::list <Symbol> _referent;	

	    /** The list of generator produced by the search */
	    std::vector <generator::Generator> _list;

	    std::vector <std::map <std::string, generator::Generator> > _symbols;
	    
	private :

	    /** 
	     * Does nothing special
	     * It is private for homogeneity reason
	     * We wan't all class to be initialized the same way
	     */
	    Visitor ();

	public :

	    /**
	     * \brief Create an empty visitor
	     */
	    static Visitor init ();

	    /**
	     * \brief Validate a symbol 
	     * \brief Traverse all inner declaration and validate them
	     * \brief If no errors occurs the returned list of generators are ready for code generation
	     */	    
	    void validate (const semantic::Symbol & sym);


	    /**
	     * \brief Validate a module
	     */
	    void validateModule (const semantic::Module & mod);


	    /**
	     * \brief Validate a function
	     */
	    void validateFunction (const semantic::Function & func);

	    /**
	     * \brief Validate a global var declaration 
	     */
	    void validateVarDecl (const semantic::VarDecl & vardecl);

	    /**
	     * \brief validate an expression, that produce a type
	     * \return a tree containing the resulting type
	     */
	    generator::Generator validateType (const syntax::Expression & type);

	    /**
	     * \brief Validate the var using it as a type
	     */
	    generator::Generator validateTypeVar (const syntax::Var & var);

	    /**
	     * \brief validate an expression, that produce a value
	     * \return a tree containing the result of the value
	     */
	    generator::Generator validateValue (const syntax::Expression & value);

	    /**
	     * \brief Validate a block of expression
	     * \return a tree containing the result of the block
	     */
	    generator::Generator validateBlock (const syntax::Block & block);

	    /**
	     * \brief Validate an fixed const integer value
	     */
	    generator::Generator validateFixed (const syntax::Fixed & fixed);	    

	    /**
	     * \return the list of generator produced by semantic validation
	     */
	    const std::vector <generator::Generator> & getGenerators () const;

	private :

	    void insertNewGenerator (const generator::Generator & generator);

	    void enterBlock ();

	    void insertLocal (const std::string & name, const generator::Generator & local);
	    
	    void quitBlock ();
	    
	};
	       
    }
    
}
