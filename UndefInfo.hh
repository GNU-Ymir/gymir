#pragma once

#include "TypeInfo.hh"

namespace Semantic {

    struct UndefInfo : TypeInfo {

	UndefInfo () {}

	virtual std::string typeToString () const {
	    return "undef";
	}

	virtual bool Is (TypeEnum type) {
	    return type == UNDEF;
	}
	   
	virtual TypeInfo * binaryOp (Lexical::TokenPtr, Syntax::Expression*) {
	    return NULL;
	}
	
	virtual TypeInfo * binaryOpRight (Lexical::TokenPtr, Syntax::Expression*) {
	    return NULL;
	}
	
	virtual TypeInfo * unaryOp (Lexical::TokenPtr) {
	    return NULL;
	}
	
	virtual TypeInfo * multOp (Lexical::TokenPtr, Syntax::Expression*) {
	    return NULL;
	}

	virtual TypeInfo * clone () override {
	    return new UndefInfo ();
	}
	
    };
    

}
