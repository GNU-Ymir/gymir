#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/_.hh>
#include <ymir/semantic/Symbol.hh>
#include <ymir/utils/Match.hh>

namespace semantic {

    /**
     * \struct Visitor
     * This class is a semantic visitor 
     * It will traverse an abstract syntax tree and generate a new tree 
     * All inner values will be typed 
     * The final tree possess only simple elements that can be directly translated into GCC internals gimple
     */

    class Visitor {
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
	
    };
    
}
