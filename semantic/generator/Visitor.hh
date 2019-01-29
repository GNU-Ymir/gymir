#pragma once

#include <ymir/tree/_.hh>
#include <ymir/semantic/generator/_.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Visitor
	 * This last visitor is the final visitor
	 * It will traverse all the generators produced by semantic analyses and translate them into gimple
	 * This visitor is called at lint time (lint meaning intermediate language)
	 * This part is the only part that is GCC dependent
	 */
	class Visitor {

	    /** The list of var decl for each blocks */
	    std::vector <generic::TreeChain> stackVarDeclChain;

	    /** The list of block currently entered */
	    std::vector <generic::BlockChain> stackBlockChain;	    	    

	    /** The global context of the generation */
	    generic::Tree _globalContext;

	    /** The global context of the generation */
	    generic::Tree _currentContext;

	    /** The declaration of the local var for each frame */
	    std::vector <std::map <uint, generic::Tree> > _declarators;
	    
	private :

	    Visitor ();

	public :

	    /**
	     * Does nothing special
	     * It is private for homogeneity reason
	     * We wan't all class to be initialized the same way	     
	     */
	    static Visitor init () ;

	    /**
	     * Finalize the generation 
	     * Declare the remaining symbol into GCC memory
	     * (global declaration needs to be dones at the end ?)
	     */
	    void finalize ();	    
	    
	    /**
	     * \brief Generate gimple tree from a generator
	     * \brief Traverse all inner generator 
	     */
	    void generate (const Generator & gen);

	    /**
	     * \brief Generate a new symbol from a global var generator
	     * \param var the variable to generate
	     */
	    void generateGlobalVar (const GlobalVar & var);

	    /**
	     * \brief Generate a new frame from a frame generator
	     * \param frame the frame to generate
	     */
	    void generateFrame (const Frame & frame);
	    
	    /**
	     * \brief Generate the declaration of a param var
	     * \return the declaration
	     */
	    generic::Tree  generateParamVar (const ParamVar & var);

	    /**
	     * \brief Transform a type from generator to gimple
	     */
	    generic::Tree generateType (const Generator & gen);

	    /**
	     * \brief Generate a tree representation of an integer type
	     */
	    generic::Tree generateIntegerType (const Integer & type);

	    /**
	     * \brief Transform a value to gimple
	     */
	    generic::Tree generateValue (const Generator & gen);

	    /**
	     * \brief Transform a block into gimple
	     */
	    generic::Tree generateBlock (const Block & bl);

	    /**
	     * \brief Transform a block into gimple
	     */
	    generic::Tree generateSet (const Set & set);

	    /**
	     * \brief Transform a fixed value into gimple
	     */
	    generic::Tree generateFixed (const Fixed & fixed);

	    /**
	     * \brief Transform a fixed value into gimple
	     */
	    generic::Tree generateBool (const BoolValue & b);

	    /**
	     * \brief Transform a binary int generator into gimple
	     */
	    generic::Tree generateBinaryInt (const BinaryInt & bin);

	    /**
	     * \brief Transform a binary int generator into gimple
	     */
	    generic::Tree generateBinaryBool (const BinaryBool & bin);

	    /**
	     * \brief Transform a var ref into gimple
	     */
	    generic::Tree generateVarRef (const VarRef & var);

	    /**
	     * \brief Transform a var decl into gimple
	     */
	    generic::Tree generateVarDecl (const VarDecl & var);
	    
	private :

	    /**
	     * Enter a new block 
	     */
	    void enterBlock () ;

	    /** 
	     * Quit the last block and return its symbol mapping 
	    */
	    generic::TreeSymbolMapping quitBlock (const lexing::Word & loc, const generic::Tree &);

	    /**
	     * Enter a new Frame
	     */
	    void enterFrame ();

	    /**
	     * Close the lastFrame 
	     */
	    void quitFrame ();

	    /**
	     * \return the tree reprensenting the global context of definition
	     */
	    const generic::Tree & getGlobalContext ();
	    
	    /**
	     * \return the tree reprensenting the current context of definition
	     */
	    const generic::Tree & getCurrentContext () const;

	    /**
	     * \brief Change the current context for definition
	     */
	    void setCurrentContext (const generic::Tree & tr);

	    /**
	     * \brief Add a new vardecl for future var referencing 
	     */
	    void insertDeclarator (uint id, const generic::Tree & decl);

	    /**
	     * \brief Get a var declarator
	     */
	    generic::Tree getDeclarator (uint id);
	    
	};

    }
    
}
