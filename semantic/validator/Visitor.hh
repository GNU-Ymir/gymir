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

	    std::vector <std::set <std::string> > _usedSyms;
	    
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
	     * \brief Validate a decorated type
	     */
	    generator::Generator validateTypeDecorated (const syntax::DecoratedExpression & type);
	    
	    /**
	     * \brief Validate the var using it as a type
	     */
	    generator::Generator validateTypeVar (const syntax::Var & var);

	    /**
	     * \brief Validate an array allocation as a type
	     */
	    generator::Generator validateTypeArrayAlloc (const syntax::ArrayAlloc & alloc);

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
	     * \brief Validate a set of expression
	     * \return a tree containing the result of the block
	     */
	    generator::Generator validateSet (const syntax::Set & set);
	    
	    /**
	     * \brief Validate an fixed const integer value
	     */
	    generator::Generator validateFixed (const syntax::Fixed & fixed);	    

	    /**
	     * \brief Validate a const bool value
	     */
	    generator::Generator validateBool (const syntax::Bool & b);

	    /**
	     * \brief Validate a const floating value
	     */
	    generator::Generator validateFloat (const syntax::Float & f);

	    /**
	     * \brief Validate a literal char
	     */
	    generator::Generator validateChar (const syntax::Char & c);
	    
	    /**
	     * \brief Validate a binary expression 
	     * \brief This generation is a bit complex as it depends on the type of the operands
	     * \brief All binary operations are handled into BinaryVisitor class
	     */
	    generator::Generator validateBinary (const syntax::Binary & bin);
	    
	    /**
	     * \brief Validate a var 
	     * \brief It will check all the local reference
	     * \brief If no local reference are found, it will find the symbol inside the table of the current frame
	     * \brief And then produce the generator for this symbol
	     */
	    generator::Generator validateVar (const syntax::Var & var);

	    /**
	     * \brief Validate a var declaration inside a block (or a frame)
	     */
	    generator::Generator validateVarDeclValue (const syntax::VarDecl & decl);
	    
	    /**
	     * \brief Validate a decorated expression
	     */
	    generator::Generator validateDecoratedExpression (const syntax::DecoratedExpression & dec_expr);

	    /**
	     * \brief Validate an if expression 
	     * \return as always a generator 
	     * \warning the generator is not always a if expression, for optimization purpose (such as the test is always false, ...)
	     */
	    generator::Generator validateIfExpression (const syntax::If & _if);

	    /**
	     * \brief Validate a list, it could be either : 
	     * \brief - an array
	     * \brief - a tuple
	     * \brief - an array type
	     * \brief - a tuple type
	     */
	    generator::Generator validateList (const syntax::List & list);

	    /**
	     * \brief Validate intricisics, it could be either :  
	     * \brief - a copy
	     * \brief - an expand
	     * \brief - type informations ...
	     */
	    generator::Generator validateIntrinsics (const syntax::Intrinsics & intr);

	    /**
	     * \brief Validate the copy intrinsics
	     */
	    generator::Generator validateCopy (const syntax::Intrinsics & intr);
	    
	    /**
	     * \brief validate an array literal
	     */
	    generator::Generator validateArray (const syntax::List & list);
	    
	    
	    /**
	     * \return the list of generator produced by semantic validation
	     */
	    const std::vector <generator::Generator> & getGenerators () const;

	    /**
	     * \brief this function is called each time a copy is performed
	     * \param type the type result of the copy 
	     * \param gen the generator that will produce the affectation 
	     * \brief This function verify that the mutability of gen is preserved
	     * \brief And that no implicit operation are performed
	     */
	    void verifyMemoryOwner (const generator::Generator & type, const generator::Generator & gen);
	    
	private :

	    void insertNewGenerator (const generator::Generator & generator);

	    void enterBlock ();

	    void insertLocal (const std::string & name, const generator::Generator & local);

	    const generator::Generator & getLocal (const std::string & name) ;
	    
	    void quitBlock ();
	    
	    /**
	     * \brief execute the content of the generator in order to retreive the compile time value 
	     */
	    generator::Generator retreiveValue (const generator::Generator & gen);

	};
	       
    }
    
}
