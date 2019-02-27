#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/_.hh>
#include <ymir/semantic/Symbol.hh>
#include <ymir/utils/Match.hh>
#include <ymir/syntax/expression/VarDecl.hh>
#include <list>

namespace semantic {

    namespace declarator {
    
	/**
	 * \struct Visitor
	 * This class is a semantic visitor 
	 * It will traverse an abstract syntax tree and generate a new tree 
	 * All inner values will be typed 
	 * The final tree possess only simple elements that can be directly translated into GCC internals gimple

	 * Their is two different phase of processing for each module : 
	 * - Declaration of all symbols
	 *   - This first phase does not produce any type checking
	 *   - It just fill the table of all scope (module part only)
	 *   - The scope of inner expression is filled at second phase

	 * - Validation of all symbols
	 *   - Compiling the cores of the function 
	 *   - Verifiying the structures, and other types 
	 *   - ...

	 * The second phase can cause another visit to an external module
	 */
	class Visitor {

	    /**
	     * The context of the visitor determine if symbols should generate code or not
	     * For exemple when visiting a function in extern context : 
	     * we do not care about the body of the function and don't create any frame
	     */
	    enum class Context {
		INNER,
		EXTERN
	    };
       
	    Context _context = Context::INNER;

	    std::list <Symbol> _referent;

	    static std::set <std::string> __imported__;
	    
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
	     * \brief Transform a AST into a semantic tree
	     */
	    semantic::Symbol visit (const syntax::Declaration & ast);

	    /**
	     * \brief Transform a syntax::module into a semantic tree
	     */
	    semantic::Symbol visitModule (const syntax::Module & mod);

	    /**
	     * \brief Create the sub module when creating a global module with a complexe path
	     */
	    void createSubModules (const lexing::Word & loc, const std::vector <std::string> & names, semantic::Symbol & last);
	    
	    /**
	     * \brief Transform a function into a semantic tree
	     */
	    semantic::Symbol visitFunction (const syntax::Function & func);

	    /**
	     * \brief Transform a struct into a semantic tree
	     */
	    semantic::Symbol visitStruct (const syntax::Struct & str);

	    /**
	     * \brief Transform an alias into a semantic tree
	     */
	    semantic::Symbol visitAlias (const syntax::Alias & stal);

	    /**
	     * \brief Transform a list of declaration into semantic tree 
	     * \return always an empty symbol
	     */
	    semantic::Symbol visitBlock (const syntax::DeclBlock & block);

	    /**
	     * \brief Transform a class declarator into a semantic tree
	     */
	    semantic::Symbol visitClass (const syntax::Class & cls);

	    /**
	     * \brief Visit an enum declaration
	     */
	    semantic::Symbol visitEnum (const syntax::Enum & stenm);

	    /**
	     * \brief Transform a vardecl to a semantic tree
	     */
	    semantic::Symbol visitVarDecl (const syntax::VarDecl & decl);

	    /**
	     * \brief Transform a global var declaration into semantic tree
	     */
	    semantic::Symbol visitGlobal (const syntax::Global & glob);
	    
	    /**
	     * \brief Transform an import into a semantic tree
	     */
	    semantic::Symbol visitImport (const syntax::Import & imp);

	private :

	    /**
	     * \brief Push a referent symbol
	     * \brief This symbol is the current symbol table that will be filled
	     * \brief It is also the symbol table used to get the symbol list
	     */
	    void pushReferent (const Symbol & sym);

	    /**
	     * \brief Remove the last referent symbol (if any)
	     * \brief If there is no referent symbol raise an internal error
	     */
	    Symbol popReferent ();

	    /** 
	     * \brief Return the current referent
	     */
	    Symbol & getReferent ();

	};

    }
}
