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
	Expression value;
	
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
	
	Instruction instruction () override;

	Expression expression () override;
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	IBlock* block ();

	IBlock* blockWithoutEnter ();

	std::vector <Instruction> & getInsts ();
	
	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Ymir::Tree toGeneric () override;

	Ymir::Tree toGenericExpr (semantic::InfoType & type, Ymir::Tree & res);

	Ymir::Tree toGenericValue ();
	
	Expression getLastExpr ();


	static const char * id () {
	    return TYPEID (IBlock);
	}
	
	virtual std::vector <std::string> getIds () override {
	    auto ids = IInstruction::getIds ();
	    ids.push_back (TYPEID (IBlock));
	    return ids;
	}
	
	void print (int nb = 0) override;

	virtual ~IBlock ();
	
    };

    typedef IBlock* Block;
    
}
