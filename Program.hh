#pragma once

#include "Ast.hh"
#include "Declaration.hh"
#include <vector>

namespace Syntax {
       
    struct Program : Ast {
	std::vector <DeclarationPtr> elems;
	Program (Lexical::TokenPtr token, std::vector<DeclarationPtr> elems)
	    : Ast (token, AstEnums::PROGRAM),
	      elems (elems)
	{}
	
	virtual void print (int nb = 0);
    };

};
