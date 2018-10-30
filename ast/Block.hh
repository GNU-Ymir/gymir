#pragma once

#include "Expression.hh"
#include <ymir/utils/Array.hh>
#include <stdio.h>
#include <list>
#include "../errors/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {
    
    class IDeclaration;
    typedef IDeclaration* Declaration;

    /**
     * \struct IBlock
     * Syntaxic node representation of a block
     * \verbatim
     block := '{' (instruction | declaration | 'on' 'exit' '=>' block)* '}'
     \endverbatim 
     */
    class IBlock : public IExpression {

	Word _ident;
	std::vector <Declaration> _decls;
	std::vector <Instruction> _insts;
	std::vector <Instruction> _preFinally;
	std::vector <IBlock*> _finally;
	std::vector <Var> _inlines;
	Expression _value;

	static std::list <IBlock*> __currentBlock__;
	
    public :

	/**
	 * \param word the location of the block
	 * \param decls the declaration done inside the block
	 * \param insts the instructions of the block
	 */
	IBlock (Word word, std::vector <Declaration> decls, std::vector <Instruction> insts) :
	    IExpression (word),
	    _decls (decls),
	    _insts (insts)
	{
	    this-> _ident.setEof ();
	}

	/**
	 * \return the location of the block
	 */
	Word& getIdent ();

	/**
	 * \brief Add finally instruction at the block exit
	 * \param insts a block of instruction
	 */
	void addFinally (IBlock * insts);

	/**
	 * \brief Add a finally an already semantic analysed instruction 
	 * \param inst an instruction that already been typed
	 */
	void addFinallyAtSemantic (Instruction inst);

	/**
	 * ??
	 */
	void addInline (Var var);
	
	Instruction instruction () override;

	Expression expression () override;
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	/**
	 * \brief alias of instruction ()
	 */
	IBlock* block ();

	/**
	 * \brief semantic analyse of the block without entering a new scope
	 */
	IBlock* blockWithoutEnter ();

	/**
	 * \return the current block (at lint time)
	 */
	static IBlock* getCurrentBlock ();

	/**
	 * \return the list of instruction performed in the block
	 */
	std::vector <Instruction> & getInsts ();
	
	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Ymir::Tree toGeneric () override;

	/**
	 * \brief Generate the generic code, when the block is used as an expression
	 * \param type the type of the block
	 * \param res the result (returned)
	 */
	Ymir::Tree toGenericExpr (semantic::InfoType & type, Ymir::Tree & res);

	Ymir::Tree toGenericValue ();

	/**
	 * \return the last expression of the block (forming the value of the block)
	 */
	Expression getLastExpr ();

	std::string prettyPrint ();
		
	static const char * id () {
	    return TYPEID (IBlock);
	}
	
	virtual std::vector <std::string> getIds () override {
	    auto ids = IExpression::getIds ();
	    ids.push_back (TYPEID (IBlock));
	    return ids;
	}
	
	void print (int nb = 0) override;

	virtual ~IBlock ();
	
    };

    typedef IBlock* Block;
    
}
