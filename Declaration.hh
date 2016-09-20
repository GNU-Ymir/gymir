#pragma once

#include "Ast.hh"

namespace Syntax {
        
    struct Declaration : Ast {

	Declaration (Lexical::TokenPtr ptr, AstEnums::AstEnum type)
	    : Ast (ptr, type)
	{}
	
	Declaration (Lexical::TokenPtr ptr)
	    : Ast(ptr, AstEnums::DECLARATION)
	{}
	
	virtual void print (int nb = 0);
    };

    typedef Declaration * DeclarationPtr;

};
