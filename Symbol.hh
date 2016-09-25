#pragma once

#include "Token.hh"
#include "TypeInfo.hh"

namespace Semantic {

    struct Symbol {
	Lexical::TokenPtr token;
	TypeInfo * type;

	Symbol (Lexical::TokenPtr tok, TypeInfo * info)
	    : token (tok),
	      type (info)
	{}

	static Symbol & empty () {
	    return empty_sym;
	}
	
	bool isVoid () {
	    return type == NULL && token->getId() == Lexical::Token::EOF_TOKEN();
	}

    private:
	
	static Symbol empty_sym;
	
    };
    
}
