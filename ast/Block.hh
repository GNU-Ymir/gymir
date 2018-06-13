#pragma once

#include "Expression.hh"
#include <ymir/utils/Array.hh>
#include <stdio.h>
#include "../errors/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {
    
    class IDeclaration;
    typedef IDeclaration* Declaration;
    
    class IBlock : public IExpression {

	Word ident;
	std::vector <Declaration> decls;
	std::vector <Instruction> insts;
	std::vector <IBlock*> finally;
	std::vector <Var> inlines;
	Expression value;

	static std::vector <IBlock*> currentBlock;
	
    public :

	IBlock (Word word, std::vector <Declaration> decls, std::vector <Instruction> insts) :
	    IExpression (word),
	    decls (decls),
	    insts (insts)
	{
	    this-> ident.setEof ();
	}

	Word& getIdent ();

	void addFinally (IBlock * insts);

	void addInline (Var var);
	
	Instruction instruction () override;

	Expression expression () override;
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	IBlock* block ();

	IBlock* blockWithoutEnter ();

	static IBlock* getCurrentBlock ();
	
	std::vector <Instruction> & getInsts ();
	
	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Ymir::Tree toGeneric () override;

	Ymir::Tree toGenericExpr (semantic::InfoType & type, Ymir::Tree & res);

	Ymir::Tree toGenericValue ();
	
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
