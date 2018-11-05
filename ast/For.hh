#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    /**
     * \struct IFor
     * The syntaxic node representation of a for loop
     * \verbatim
     for := 'for' (':' Identifier) var (',' var)* 'in' expression block
     \endverbatim 
    */
    class IFor : public IInstruction {

	/** The identifier of the loop (may be Word::eof ()) */
	Word _id;

	/** the variable used to iter */
	std::vector <Var> _var;

	/** The expression that will be traverse */
	Expression _iter;

	/** The instructions to run at each loop */
	Block _block;

	/** The const decoration of the iterable variables */
	std::vector <bool> _const;

	/** the infotype of the iter var, after ApplyOp */
	semantic::InfoType _ret;
	
    public:

	/**
	 * \param token the location of the loop
	 * \param id the identifier of the loop
	 * \param var the array of variable used to iter
	 * \param iter the expression on which we will iter
	 * \param bl the block of instructions 
	 * \param _const the decoration of the iter's variables
	 */
	IFor (Word token, Word id, const std::vector <Var>& var, Expression iter, Block bl, std::vector <bool> _const);

	Instruction instruction () override;
	
	Instruction templateReplace (const std::map <std::string, Expression>&) override;
	
	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual ~IFor ();

    private :

	/** 
	 * \brief Transform the for loop into a sequence of instruction (cte for only)
	 * \brief If it is not possible errors are thrown, and NULL is returned
	 */
	Instruction immutable (Expression expr);

	/**
	 * \brief Compile time execution of a for loop on a range
	 */
	Instruction immutableRange (std::vector <Var> & vars, Expression expr);

	/**
	 * \brief Compile time execution of a for loop on a string
	 */
	Instruction immutableString (std::vector <Var> & vars, Expression expr);

	/**
	 * \brief Compile time execution of a for loop on a tuple
	 */
	Instruction immutableTuple (std::vector <Var> & vars, Expression expr);

	/**
	 * \brief Compile time execution of a for loop on a fake tuple (variadic parameter)
	 */
	Instruction immutableTupleFake (std::vector <Var> & vars, Expression expr);	

	/**
	 * \brief Compile time execution of a for loop on a macro expression
	 */
	Instruction immutableMacro (std::vector <Var> & vars, Expression expr);

	/**
	 * \brief Find the different mandatory functions to override a for loop 
	 */
	Instruction findOpApply (); 
	
    };

    typedef IFor* For;
    
}
