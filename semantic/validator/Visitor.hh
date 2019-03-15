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
	    
	    /** The list of loop breaks types (when inside a loop) cf enterLoop and quitLoop */
	    std::list <generator::Generator> _loopBreakTypes;
	    
	    std::vector <std::vector <std::set <std::string> > > _usedSyms;
	    
	    std::vector <std::vector <std::map <std::string, generator::Generator> > > _symbols;

	    /** The list of declared and usable structures */
	    std::vector <semantic::Symbol>     _structSyms;
	    std::vector <generator::Generator> _structs;
	    
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
	     * \brief Validate a struct declaration
	     * \brief unlike, function or var decl, this will not create any generator, but just check the integrity of the structure
	     */
	    generator::Generator validateStruct (const semantic::Symbol & str);
	    
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
	     * \brief Validate a slice type from an array literal
	     */
	    generator::Generator validateTypeSlice (const syntax::List & array);
	    
	    /**
	     * \brief Validate a tuple type from a tuple literal
	     */
	    generator::Generator validateTypeTuple (const syntax::List & tuple);	    
	    
	    /**
	     * \brief validate an expression, that produce a value
	     * \brief If the value is a breaker or a returner throw an error
	     * \return a tree containing the result of the value
	     */
	    generator::Generator validateValue (const syntax::Expression & value);

	    /**
	     * \brief validate an expression, that produce a value
	     * \return a tree containing the result of the value
	     */
	    generator::Generator validateValueNoReachable (const syntax::Expression & value);

	    /**
	     * \brief Validate a block of expression
	     * \return a tree containing the result of the block
	     */
	    generator::Generator validateBlock (const syntax::Block & block);

	    /**
	     * \brief Validate an inner declaration of a module
	     * \param decl the module to declare, and validate
	     */
	    semantic::Symbol validateInnerModule (const syntax::Declaration & decl);
	    
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
	     * \brief All binary operations are handled by BinaryVisitor class
	     */
	    generator::Generator validateBinary (const syntax::Binary & bin);

	    /**
	     * \brief Validate a unary expression
	     * \brief All unary operations are handled by UnaryVisitor class
	     */
	    generator::Generator validateUnary (const syntax::Unary & un);
	    
	    /**
	     * \brief Validate a var 
	     * \brief It will check all the local reference
	     * \brief If no local reference are found, it will find the symbol inside the table of the current frame
	     * \brief And then produce the generator for this symbol
	     */
	    generator::Generator validateVar (const syntax::Var & var);

	    /**
	     * \brief Transform global extern symbol into valid generators
	     * \param loc the location of the reference to those symbols
	     * \param multSym the list of symbols
	     */
	    generator::Generator validateMultSym (const lexing::Word & loc, const std::vector <Symbol> & multSym);

	    /**
	     * \brief Transform global extern symbol into valid generators
	     * \param loc the location of the reference to those symbols
	     * \param multSym the list of symbols
	     */
	    generator::Generator validateMultSymType (const lexing::Word & loc, const std::vector <Symbol> & multSym);  
	    
	    /**
	     * \brief Validate the prototype of a function in order to refer to it
	     * \param func the function prototype to validate
	     */
	    generator::Generator validateFunctionProto (const semantic::Function & func);
	    
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
	     * \brief Validate a while expression 
	     * \return as always a generator 
	     */
	    generator::Generator validateWhileExpression (const syntax::While & _wh);

	    /**
	     * \brief Validate a break expression
	     */
	    generator::Generator validateBreak (const syntax::Break & br);
	    
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
	     * \brief Validate the alias intrinsics
	     */
	    generator::Generator validateAlias (const syntax::Intrinsics & intr);
	    
	    /**
	     * \brief Validate a mult operator
	     * \brief A mult operator is an operator with one left operand and multiple right operand
	     * \brief It can be either :
	     * \brief - Brackets
	     * \brief - Parentheses
	     */
	    generator::Generator validateMultOperator (const syntax::MultOperator & mult);

	    
	    /**
	     * \brief validate an array literal
	     */
	    generator::Generator validateArray (const syntax::List & list);

	    /**
	     * \brief validate an tuple literal
	     */
	    generator::Generator validateTuple (const syntax::List & list);
	    
	    
	    /**
	     * \return the list of generator produced by semantic validation
	     */
	    const std::vector <generator::Generator> & getGenerators () const;

	    /**
	     * \brief this function is called each time a copy is performed
	     * \param loc the location of the affectation
	     * \param type the type result of the copy 
	     * \param gen the generator that will produce the affectation 
	     * \param construct is this a construction ? (ref are not affected yet)
	     * \brief This function verify that the mutability of gen is preserved
	     * \brief And that no implicit operation are performed
	     */
	    void verifyMemoryOwner (const lexing::Word & loc, const generator::Generator & type, const generator::Generator & gen, bool construct);
	    
	private :

	    void enterForeign ();

	    void exitForeign ();

	    
	    /**
	     * \brief Insert a new Generator that has passed the semantic validation
	     * \brief All the symbol passed here, will be transformed at generation time
	     * \param generator the valid generator
	     */
	    void insertNewGenerator (const generator::Generator & generator);

	    /**
	     * \brief Enter a new scope
	     */
	    void enterBlock ();

	    /**
	     * \brief insert a new symbol in the frame local scope
	     * \param name the name of the symbol
	     * \param local the symbol 
	     */
	    void insertLocal (const std::string & name, const generator::Generator & local);

	    /**
	     * \brief Get a localy declared symbol
	     * \param name the name of the symbol to get
	     */
	    const generator::Generator & getLocal (const std::string & name) ;

	    /**
	     * \brief Exit a scope
	     */
	    void quitBlock ();

	    /**
	     * \brief Ignore all the local var declared in the current block
	     * \return all the local var discarded
	     */
	    std::map <std::string, generator::Generator> discardAllLocals ();
	    
	    /** 
	     * Enter a new breakable loop 
	     */
	    void enterLoop ();
	    
	    /** 
	     * quit a breakable loop
	     * \return the type of the inner breakers
	     */
	    generator::Generator quitLoop ();
	    
	    /**
	     * \return the loop type
	     */
	    const generator::Generator & getCurrentLoopType () const;

	    /**
	     * \brief Change the type of the current loop
	     */
	    void setCurrentLoopType (const generator::Generator & type);

	    /**
	     * \return !this-> _loopBreakTypes.empty ()
	     */
	    bool isInLoop () const;
	    
	    /**
	     * \brief execute the content of the generator in order to retreive the compile time value 
	     */
	    generator::Generator retreiveValue (const generator::Generator & gen);

	    /**
	     * \brief Retreive a globally declared symbol (outside of the frame, or inner declared but not local just private)
	     * \param name the name of the symbol to retreive
	     */
	    std::vector <Symbol> getGlobal (const std::string & name);
	    
	private :

	    void verifyRecursivity (const lexing::Word & loc, const generator::Generator & gen, const Symbol & sym) const;

	    /**
	     * \brief Create the main function symbol that will call the actual main of the program
	     */
	    void createMainFunction (const lexing::Word & loc);
	    
	};
	       
    }
    
}
