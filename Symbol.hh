#pragma once

#include "Token.hh"
#include "TypeInfo.hh"
#include <tr1/memory>
#include <gc/gc_cpp.h>

namespace Semantic {
    
    struct Symbol : gc {
	Lexical::TokenPtr token;
	TypeInfo * type;

	Symbol (Lexical::TokenPtr tok, TypeInfo * info)
	    : token (tok),
	      type (info)
	{}

	bool isVoid () {
	    return this-> type == NULL;
	}

	Ymir::Tree treeDecl () {
	    return this-> decl;
	}

	void treeDecl (Ymir::Tree t) {
	    this-> decl = t;
	}

	static Symbol* empty () {
	    return _empty;
	}

	std::string toString () {
	    return std::string ("Sym (") + this-> token-> getStr () + ")";
	}
	
    private:
	static Symbol* _empty;
	
	Ymir::Tree decl;
    };

    typedef Symbol* SymbolPtr;
    
    
}
