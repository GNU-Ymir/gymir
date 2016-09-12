#pragma once

#include "Token.hh"

namespace Syntax {


    struct Ast {
	
	Lexical::TokenPtr token;
	Ast (Lexical::TokenPtr token) : token (token) {}			
	
    };
    
    typedef std::tr1::shared_ptr<Ast> AstPtr;	           

}
