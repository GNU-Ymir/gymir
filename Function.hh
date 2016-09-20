#pragma once

#include "Declaration.hh"
#include "Var.hh"
#include "Block.hh"

namespace Syntax {

    struct Function : Declaration {
	
	VarPtr type = Var::empty ();
	std::vector <VarPtr> params;
	BlockPtr block;
	
	Function (Lexical::TokenPtr info, VarPtr type, std::vector<VarPtr> params, BlockPtr block)
	    : Declaration (info, AstEnums::FUNCTION),
	      type (type),
	      params (params),
	      block (block)
	{}

	    Function (Lexical::TokenPtr info, std::vector<VarPtr> params, BlockPtr block)
		: Declaration (info, AstEnums::FUNCTION),
	      params (params),
	      block (block)
	{}
	
	virtual void print (int nb = 0);
	
    };


};
