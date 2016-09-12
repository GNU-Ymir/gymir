#pragma once

#include "Syntax.hh"
#include "Expression.hh"
#include "Instruction.hh"


namespace Syntax {

    struct Declaration : Ast {
	Declaration (Lexical::TokenPtr ptr) : Ast(ptr) {}
    };

    typedef std::tr1::shared_ptr<Declaration> DeclarationPtr;
    
    struct Function : Declaration {

	VarPtr type = Var::empty ();
	std::vector <VarPtr> params;
	BlockPtr block;

	Function (Lexical::TokenPtr info, VarPtr type, std::vector<VarPtr> params, BlockPtr block)
	    : Declaration (info),
	      type (type),
	      params (params),
	      block (block) {}

	    Function (Lexical::TokenPtr info, std::vector<VarPtr> params, BlockPtr block)
	    : Declaration (info),
	      params (params),
	      block (block) {}	      	
	
    };

    
};
