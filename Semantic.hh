#pragma once

#include "Token.hh"

namespace Semantic {

    struct Ast {
	Lexical::TokenPtr token;
	Ast (Lexical::TokenPtr token) : token (token) {}
    };

    Ast empty ();
    
};
