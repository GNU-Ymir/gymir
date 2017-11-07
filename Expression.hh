#pragma once

#include "Instruction.hh"
#include "Symbol.hh"
#include "Tree.hh"

namespace Syntax {

    struct Expression : Instruction {
	Expression (Lexical::TokenPtr ptr, AstEnums::AstEnum type)
	    : Instruction (ptr, type)
	{}
	
	Expression (Lexical::TokenPtr ptr)
	    : Instruction (ptr, AstEnums::EXPRESSION)
	{}

	Semantic::SymbolPtr getType () const {
	    return this-> sym;
	}
	
	Instruction * instruction () final;
	
	virtual Ymir::Tree statement ();
	
	virtual Ymir::Tree treeExpr ();
	
	virtual Expression * expression ();
	
	virtual void print (int nb = 0);

	virtual ~Expression () {}

	Semantic::SymbolPtr sym = Semantic::Symbol::empty ();
	
    protected:
			
    };

    typedef Expression* ExpressionPtr;

}
