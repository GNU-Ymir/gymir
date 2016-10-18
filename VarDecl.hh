#pragma once

#include "Expression.hh"
#include "Var.hh"
#include "Instruction.hh"
#include <vector>

namespace Syntax {

    struct VarDecl : Instruction {

	std::vector <VarPtr> decls;
	std::vector <ExpressionPtr> affects;
	
	VarDecl (Lexical::TokenPtr ptr, std::vector<VarPtr> decls, std::vector<ExpressionPtr> insts) 
	    : Instruction (ptr, AstEnums::VAR_DECL),
	      decls (decls),
	      affects (insts)
	{}

	VarDecl (VarDecl * other) 
	    : Instruction (other-> token, AstEnums::VAR_DECL),
	      decls (other-> decls),
	      affects (other-> affects)
	{}

	virtual InstructionPtr instruction ();

	virtual void print (int nb = 0);		

    };

    typedef VarDecl * VarDeclPtr;

};
