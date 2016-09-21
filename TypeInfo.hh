#pragma once

#include "Tree.hh"
#include "Token.hh"
#include <vector>

namespace Syntax {
    struct Expression;
}

namespace Semantic {
    
    enum TypeEnum {
	INT,
	FLOAT,
	CHAR	
    };

    struct TypeInfo {

	TypeInfo ();
	
	Ymir::Tree buildBinaryOp (Syntax::Expression* left, Syntax::Expression* right);	
	Ymir::Tree buildUnaryOp (Syntax::Expression* elem);
	Ymir::Tree buildMultOp (Syntax::Expression* left, Syntax::Expression* rights);

	virtual bool Is (TypeEnum) = 0;
	virtual TypeInfo * binaryOp (Lexical::TokenPtr tok, Syntax::Expression*) = 0;
	virtual TypeInfo * binaryOpRight (Lexical::TokenPtr tok, Syntax::Expression*) = 0;
	virtual TypeInfo * unaryOp (Lexical::TokenPtr tok) = 0;
	virtual TypeInfo * multOp (Lexical::TokenPtr tok, Syntax::Expression*) = 0;

	virtual ~TypeInfo () {
	    all[id] = NULL;
	}
	
	static void clear () {
	    for (auto it : all) {
		if (it != NULL)
		    delete it;
	    }
	}

	
    protected:

	typedef Ymir::Tree (*BinopLint) (Syntax::Expression*, Syntax::Expression*);
	typedef Ymir::Tree (*UnopLint) (Syntax::Expression*);
	
	BinopLint binopFoo;
	UnopLint unopFoo;
	BinopLint multFoo;

    private:

	ulong id;	
	static std::vector <TypeInfo*> all;
	
    };
    
}
