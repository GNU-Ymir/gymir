#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"

#include "target.h"
#include "tree.h"
#include "tree-iterator.h"
#include "input.h"
#include "diagnostic.h"
#include "stringpool.h"
#include "cgraph.h"
#include "gimplify.h"
#include "gimple-expr.h"
#include "convert.h"
#include "print-tree.h"
#include "stor-layout.h"
#include "fold-const.h"

#include "convert.h"
#include "langhooks.h"
#include "ubsan.h"
#include "stringpool.h"
#include "attribs.h"
#include "asan.h"
#include "tree-pretty-print.h"
#include "print-tree.h"
#include "cppdefault.h"
#include "tm.h"

#include <ymir/lexing/Word.hh>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>

#include <vector>
#include <list>
#include <map>


namespace generic {

    /**
     * This class is a form of proxy for generic tree
     * Unlike all other proxy this one is only referencing 
     * Meaning a tree can be in multiple instance of Tree
     */
    struct Tree {

	tree _t;
	
	location_t _loc;

    private :
	
	Tree ();

    public :

	/**
	 * \brief Initialize a tree from a gimple one
	 * \param loc the location of the declaration
	 * \param t the content of the generation 
	 */
	static Tree init (const location_t & loc, const tree & t);

	/**
	 * \brief Create an error tree
	 */
	static Tree error ();
	
	/**
	 * \brief Create an empty tree (different from error)
	 */
	static Tree empty ();

	/**
	 * \brief Create a block (gimple definition of a block)
	 * \param vars the stack declaration of vars
	 * \param chain the block chain 
	 */
	static Tree block (const lexing::Word & loc, const Tree & vars, const Tree & chain);

	/**
	 * \brief Create a tree type of type void 
	 */
	static Tree voidType ();

	/**
	 * \brief Create a tree type of type bool
	 */
	static Tree boolType ();

	/**
	 * \brief Create a tree type of integer type
	 * \param size the number of bit of the type (assume its 8, 16, 32, 64 or 0)
	 * \param isSigned if true, the generated type will be capable of encoding negative numbers
	 */
	static Tree intType (int size, bool isSigned);

	/**
	 * \brief Create a pointer type
	 */
	static Tree pointerType (const Tree & inner);
	
	/**
	 * \brief Create a tree type of floating type
	 * \param size the number of bit of the type (assume its 32, 64 or 0)
	 */
	static Tree floatType (int size);

	/**
	 * \brief Create a tree type of char type
	 * \param size the number of bit of the type (assume its 8, 16, 32 or 0)
	 */
	static Tree charType (int size);
	
	/**
	 * \brief Create an static array type of size size
	 */
	static Tree staticArray (const Tree & inner, int size);

	/**
	 * \brief Create a dynamic array type
	 */
	static Tree dynArray (const Tree & inner);
	
	/**
	 * \brief Create a new var declaration 
	 * \param loc the location of the var, (frontend version)
	 * \param name the name of the var
	 * \param type the type of the var
	 */
	static Tree varDecl (const lexing::Word & loc, const std::string & name, const Tree & type);

	/**
	 * \brief Create a declaration expression
	 * \param loc the location 
	 * \param decl the declaration content (VAR_DECL, ...)
	 */
	static Tree declExpr (const lexing::Word & loc, const Tree & decl);	
	
	/**
	 * \brief Create a new parameter var declaration (for frame declaration)
	 * \param loc the location of the parameter
	 * \param name the name of the parameter
	 * \param type the type of the param
	 */
	static Tree paramDecl (const lexing::Word & loc, const std::string & name, const Tree & type);

	/**
	 * \brief Create a new function type
	 * \param retType the type returned by the function 
	 * \param params the parameter types of the function
	 */
	static Tree functionType (const Tree & retType, const std::vector <Tree> & params);

	/**
	 * \brief Create a new function declaration 
	 * \param location the location of the frame
	 * \param name the name of the frame
	 * \param type the type of the frame
	 */
	static Tree functionDecl (const lexing::Word & location, const std::string & name, const Tree & type);

	/**
	 * \brief Create a result declaration
	 * \param location the location of the declaration
	 * \param type the type of the declaration
	 */
	static Tree resultDecl (const lexing::Word & location, const Tree & type);

	/**
	 * \brief Create an affectation instruction from right to left
	 */
	static Tree affect (const lexing::Word & location, const Tree & left, const Tree & right);

	/**
	 * \brief Create a binary expression
	 * \param location the location of the operation
	 * \param code the code of the operator
	 * \param type the type of the operation
	 * \param left the left operand
	 * \param right the right operand
	 */
	static Tree binary (const lexing::Word & location, tree_code code, const Tree & type, const Tree & left, const Tree & right);
		
	/**
	 * \brief Create a compound expression (we can see that as a list of expression -> left; right)
	 */
	static Tree compound (const lexing::Word & location, const Tree & left, const Tree & right);

	/**
	 * Create an array constructor 
	 */
	static Tree constructIndexed (const lexing::Word & loc, const Tree & type, const std::vector <Tree> & values);
	
	/**
	 * \brief Generate a conditional jump
	 * \brief if the test test is true, jump to left, and jump to right otherwise
	 * \brief jump to end is perform after left and right, in any case 
	 * \verbatim
	 begin :  
	     test => goto left
	     goto right 
	 left : 
	      left 
	      goto end
	 right : 
	      right 
	      goto end
	 end:
	 \endverbatim
	 */
	static Tree conditional (const lexing::Word & location, const Tree & context, const Tree & test, const Tree & left, const Tree & right);

	/**
	 * \brief Create a conditional jump
	 */
	static Tree condExpr (const lexing::Word & loc, const Tree & test, const Tree & gotoS, const Tree & gotoF);
	
	/**
	 * \brief Create a uint cst
	 */
	static Tree buildIntCst (const lexing::Word & loc, ulong value, const Tree & type);

	/**
	 * \brief create a int cst
	 */
	static Tree buildIntCst (const lexing::Word & loc, long value, const Tree & type);

	/**
	 * \brief Create a float const
	 * \param value the value is encoded in a string to prevent approximation
	 */
	static Tree buildFloatCst (const lexing::Word & loc, const std::string & value, const Tree & type);

	/**
	 * \brief Generate a bool const
	 */
	static Tree buildBoolCst (const lexing::Word & loc, bool value);
	
	/**
	 * \brief Generate a bool const
	 */
	static Tree buildCharCst (const lexing::Word & loc, uint value, const Tree & type);

	/**
	 * \brief Generate the address pointer of value
	 * \param loc the location 
	 * \param value the value from which we wan't to get an address
	 * \param type the type of the address (assumed a pointer type)
	 */
	static Tree buildAddress (const lexing::Word & loc, const Tree & value, const Tree & type) ;	

	/**
	 * \brief Build a function call
	 */
	static Tree buildCall (const lexing::Word & loc, const Tree & type, const Tree & fn, const std::vector <Tree> & params);
	
	/**
	 * \brief Create a return stmt
	 * \param resultDecl we need the result declaration of the current frame to perform a return stmt
	 */
	static Tree returnStmt (const lexing::Word & loc, const Tree & resultDecl, const Tree & value);
	
	/**
	 * \brief Create a label
	 */
	static Tree makeLabel (const lexing::Word & loc, const Tree & context, const std::string & name);

	/**
	 * \brief create a goto label expression
	 */
	static Tree gotoExpr (const lexing::Word & loc, const Tree & label);

	/**
	 * \brief Create a label expression, that can be placed in a stmt
	 */
	static Tree labelExpr (const lexing::Word & loc, const Tree & label);
	
	/**
	 * \brief Build a tree of 1 operands
	 */
	static Tree build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1);
    
	/**
	 * \brief Build a tree of 2 operands
	 */
	static Tree build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1, const Tree & t2);

	/**
	 * \brief Build a tree of 3 operands
	 */
	static Tree build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3);

	/**
	 * \brief Build a tree of 4 operands
	 */
	static Tree build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4);

	/**
	 * \brief Build a tree of 5 operands
	 */
	static Tree build (tree_code tc, lexing::Word loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4, const Tree & t5);

	/**
	 * \return the location of the tree (for debuging infos)
	 */
	const location_t & getLocus () const;

	/**
	 * \brief set the location of the tree (for debuging infos)
	 */
	void setLocus (const location_t & loc);

	/**
	 * \brief Break the proxy, and return the generic gimple tree
	 */
	const tree & getTree () const;

	/**
	 * \brief Break the proxy, and set the generic gimple tree
	 */
	void setTree (const tree & t);

	/**
	 * \return the kind of tree 
	 */
	tree_code getTreeCode () const;

	/**
	 * \return this tree is an error tree ?
	 */
	bool isError () const;

	/**
	 * \return this tree contains nothing ? 
	 */
	bool isEmpty () const;

	/**
	 * \brief Associated to a declaration 
	 * \return is this a static symbol ?
	 */
	bool isStatic () const;
	
	/**
	 * \brief set the declaration static information
	 */
	void isStatic (bool);

	/**
	 * \brief Associated to labels
	 * \brief I don't really understand the meaning of this 
	 * \brief but, it tells if this label is used
	 */
	bool isUsed () const;

	/**
	 * \brief Set the used information of a label 
	 */
	void isUsed (bool);

	/** 
	 * \brief Associated to a declaration or a label
	 * \return this tree is externally declared
	 */
	bool isExternal () const;

	/**
	 * \brief set the external information 
	 * \brief true, means it is not declared in this tree
	 */
	void isExternal (bool);

	/**
	 * \return Tell if this declaration needs to be preserved
	 */
	bool isPreserved () const;
	
	/**
	 * \brief Set to true, means that this declaration needs to be preserved
	 */
	void isPreserved (bool);

	/**
	 * \return Tell if we can create an address from this tree
	 */
	bool isAddressable () const;

	/**
	 * \brief Set to true, means we can create an address from this tree
	 */
	void isAddressable (bool);
	
	/**
	 * \return Is this declaration public ?
	 */
	bool isPublic () const;

	/**
	 * \brief Set to true, means this symbol can be accessed from outside
	 */
	void isPublic (bool);

	/**
	 * \return the declaration context
	 */
	Tree getDeclContext () const;
	
	/**
	 * \brief Change the declaration context of this declaration
	 */
	void setDeclContext (const Tree & context);

	/**
	 * \return the initial value of the declaration 
	 */
	Tree getDeclInitial () const;

	/**
	 * \brief set the initial value of the declaration
	 */
	void setDeclInitial (const Tree & init);

	/**
	 * \brief Used within a function declaration 
	 * \brief the saved tree is the content of the function
	 */
	Tree getDeclSavedTree () const;
	
	/**
	 * \brief Used within a function declaration 
	 * \brief the saved tree is the content of the function
	 */
	void setDeclSavedTree (const Tree & saved);

	/**
	 * \brief Is this declaration a weak one ?
	 */
	bool isWeak () const;

	/**
	 * \brief Set the weak information of the function
	 */
	void isWeak (bool);

	/**
	 * \brief Applied to a type, tell if it is a scalar one
	 */
	bool isScalar () const;
	
	/**
	 * \brief Set the declaration arguments
	 * \param args a list of PARM_DECL
	 */
	void setDeclArguments (const std::list <Tree> & args);

	/**
	 * \brief Change the arg type of the declaration
	 */
	void setArgType (const Tree & type);
	
	/**
	 * \brief Set the result declaration
	 */
	void setResultDecl (const Tree & result);

	/**
	 * \brief Change the super context of a block
	 */
	void setBlockSuperContext (const Tree & context);
	
	/**
	 * \return the type of this tree
	 */
	Tree getType () const;

	/**
	 * \return this type encode a pointer type ?
	 */
	bool isPointerType () const;
	
	/**
	 * \brief Applicable to a type
	 * \return the size of the type, in number of byte
	 */
	uint getSize () const;
	
	/**
	 * \return the i eme operand of this tree
	 */
	Tree getOperand (int i) const;

	/**
	 * \return the list of operation in case of compound expr
	 * \return an empty tree otherwise
	 */
	Tree getList () const;

	/**
	 * \return the value part of a compound expr
	 * \return the tree, if it is not a compound expression
	 */
	Tree getValue () const;
	
	/**
	 * \return the value of the true, if it is a pointer value, unref it
	 */
	Tree toDirect () const;

	/**
	 * \return create a pointer unref at index 
	 */
	Tree buildPointerUnref (int index) const;
	
    };
   
    inline bool operator == (Tree t1, Tree t2) {
	return t1.getTree () == t2.getTree ();
    }

    inline bool operator != (Tree t1, Tree t2) {
	return !(t1 == t2);
    }

}
