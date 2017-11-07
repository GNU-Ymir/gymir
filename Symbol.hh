#pragma once

#include "Token.hh"
#include "TypeInfo.hh"
#include <tr1/memory>

namespace Semantic {

    struct Symbol;

    typedef std::tr1::shared_ptr <Symbol> SymbolPtr;

    struct Symbol {
	Lexical::TokenPtr token;
	TypeInfo * type;

	Symbol (Lexical::TokenPtr tok, TypeInfo * info)
	    : token (tok),
	      type (info)
	{}

	bool isVoid () {
	    return this-> type == NULL;
	}

	Ymir::Tree& treeDecl () {
	    return this-> decl;
	}

	static SymbolPtr empty () {
	    return _empty;
	}

    private:
	static SymbolPtr _empty;
	
	Ymir::Tree decl;
    };

    
}
