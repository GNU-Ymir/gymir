#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/_.hh>
#include <ymir/syntax/expression/_.hh>
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

	    std::list <Symbol> _referent;

	    static std::set <std::string> __imported__;	    

	    bool _isWeak = false;

	    bool _isTrusted = false;
	    
	private : 

	    /** 
	     * Does nothing special
	     * It is private for homogeneity reason
	     * We wan't all class to be initialized the same way
	     */
	    Visitor (bool isTrusted); 

	public :
	    
	    /**
	     * \brief Create an empty visitor
	     */
	    static Visitor init (bool isTrusted = false);

	    /**
	     * \brief Transform a AST into a semantic tree
	     */
	    semantic::Symbol visit (const syntax::Declaration & ast, bool pub);

	    /**
	     * \brief Transform a syntax::module into a semantic tree
	     */
	    semantic::Symbol visitModule (const syntax::Module & mod);

	    /**
	     * \brief Create the sub module when creating a global module with a complexe path
	     */
	    void createSubModules (const lexing::Word & loc, const std::vector <std::string> & names, semantic::Symbol last);
	    
	    /**
	     * \brief Transform a function into a semantic tree
	     */
	    semantic::Symbol visitFunction (const syntax::Function & func, bool isExtern = false, bool insert = true);

	    /**
	     * \brief Transform a constructor into a semantic tree
	     */
	    semantic::Symbol visitConstructor (const syntax::Constructor & cs);
	    
	    /**
	     * \brief Transform a struct into a semantic tree
	     */
	    semantic::Symbol visitStruct (const syntax::Struct & str, bool insert = true);

	    /**
	     * \brief Transform an alias into a semantic tree
	     */
	    semantic::Symbol visitAka (const syntax::Aka & stal);

	    /**
	     * \brief Transform a list of declaration into semantic tree 
	     * \return always an empty symbol
	     */
	    semantic::Symbol visitBlock (const syntax::DeclBlock & block);

	    /**
	     * \brief Transform a list of external declaration into semantic tree
	     * \return always an empty symbol
	     */
	    semantic::Symbol visitExtern (const syntax::ExternBlock & ex_block);
	    
	    /**
	     * \brief Transform a class declarator into a semantic tree, does not traverse the content of the class
	     */
	    semantic::Symbol visitClass (const syntax::Declaration & stdecl);
	    
	    /**
	     * \brief Visit the internal part of a class
	     */
	    void visitInnerClass (Symbol cl, const std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub);

	    /**
	     * \brief Visit the internal part of a macro
	     */
	    void visitInnerMacro (Symbol cl, const std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub);
	    
	    /**
	     * \brief Transform a trait declarator into a semantic tree
	     */
	    semantic::Symbol visitTrait (const syntax::Trait & trait);


	    /**
	     * \brief Visit the internal part of a trait
	     */
	    void visitInnerTrait (Symbol tr, const std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub);
	    
	    /**
	     * \brief Transform a trait implementation into a semantic tree, don't traverse the content of the mixin
	     */
	    semantic::Symbol visitImpl (const syntax::Declaration & impl);
	    
	    /**
	     * \brief Visit an enum declaration
	     */
	    semantic::Symbol visitEnum (const syntax::Enum & stenm);

	    /**
	     * \brief Visit a macro declaration
	     */
	    semantic::Symbol visitMacro (const syntax::Macro & mc);

	    /**
	     * \brief Visit a macro constructor declaration
	     */
	    semantic::Symbol visitMacroConstructor (const syntax::MacroConstructor & constr);

	    /**
	     * \brief Visit a macro constructor declaration
	     */
	    semantic::Symbol visitMacroRule (const syntax::MacroRule & constr);
	    
	    /**
	     * \brief Transform a vardecl to a semantic tree
	     */
	    semantic::Symbol visitVarDecl (const syntax::VarDecl & decl, const std::string & comment);

	    /**
	     * \brief Transform a global var declaration into semantic tree
	     */
	    semantic::Symbol visitGlobal (const syntax::Global & glob);
	    
	    /**
	     * \brief Transform an import into a semantic tree
	     */
	    semantic::Symbol visitImport (const syntax::Import & imp, bool pub);

	    /**
	     * \brief Transform a template declaration into semantic tree
	     */
	    semantic::Symbol visitTemplate (const syntax::Template & tep);
	    
	    /**
	     * \brief Push a referent symbol
	     * \brief This symbol is the current symbol table that will be filled
	     * \brief It is also the symbol table used to get the symbol list
	     */
	    void pushReferent (const Symbol sym);

	    /**
	     * \brief Remove the last referent symbol (if any)
	     * \brief If there is no referent symbol raise an internal error
	     */
	    Symbol popReferent ();

	    /** 
	     * \brief Return the current referent
	     */
	    Symbol getReferent ();

	    /**
	     * \brief Import all the core files in the current referent
	     */
	    void importAllCoreFiles ();

	    /**
	     * \brief All the declaration of this visitor will be weak?
	     */
	    bool isWeak () const;

	    /**
	     * From now on, all the symbol declared by this visitor will be weak
	     */
	    void setWeak ();

	    /**
	     * Clean the global symbols
	     */
	    static void purge ();
	    
	};

    }
}
